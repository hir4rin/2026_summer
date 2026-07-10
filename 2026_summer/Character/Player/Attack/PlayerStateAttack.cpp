#include "PlayerStateAttack.h"
#include "Player.h"
#include "../../../Game.h"
#include "../../../Input.h"
#include "../../AttackCol.h"
#include "../../../Managers/CollisionManager.h"
#include "../System.h"


namespace
{
	constexpr float kComboInputStart = 0.2f;//コンボ入力受付開始のアニメーションの進行率
	constexpr float kComboInputEnd = 0.8f;//コンボ入力受付終了のアニメーションの進行率

	constexpr float kPlayerCenter = 100.0f;//プレイヤーの当たり判定の中心点までのy軸の距離
}


PlayerStateAttack::PlayerStateAttack(std::weak_ptr<Player> player, AttackType type):
	PlayerState(player),m_attackType(type)
{
	//playerが既に破棄されていたら早期リターンする//trueで破棄されている
	if (m_owner.expired())return;
}

PlayerStateAttack::~PlayerStateAttack()
{
}

void PlayerStateAttack::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;
	//攻撃の方向を決める
	DetermineAttackDirection();
	//アニメーションの初期化//コンボの段数によってアニメーションを変える//-1はplayerがいないとき
	int currentComboIndex = SelectAnimInit();
	const ComboNode& node = player->m_comboChain[currentComboIndex];
	player->m_anim.ChangeAnim(node.animName, false,1.0f);
	//上下差がある攻撃の時はここで初速を与える
	if (node.moveSpeedY != 0)
	{
		player->m_vel = player->m_targetVec * node.moveSpeedX + Vector3(0, node.moveSpeedY, 0);
		//上下の速度を保存
		m_InitVel = player->m_vel;
		//上昇攻撃
		if (node.moveSpeedY > 0)
		{
			player->m_isGround = false;//ジャンプ状態にする
			player->SetIsFloor(false);
		}
	}
	float totalAnimFrame = player->m_anim.GetAnimTotalFrame(node.animName);
	//ここでColliderを生成する//あとhitstopとkAttackColOffset
	player->m_attackData = {
	.attackPower = node.attackPower,
	.knockBackPower = Vector3(node.knockBackXZ, node.knockBackY,0),
	//.knockBackPower = Vector3(0.0f,node.knockBackY,0.0f),//吹き飛ばない攻撃にする
	.knockBackFrame = totalAnimFrame,
	.hitStopTime = 0.1f,
	.kAttackColOffset = 30.0f,
	.isKirimomi = node.isKirimomi
	};
	m_attackCol = std::make_shared<AttackCol>(m_owner, player->m_attackData);
	Vector3 offset = player->m_targetVec * player->m_attackData.kAttackColOffset
					+ Vector3(0, kPlayerCenter, 0);//プレイヤーの前方に50.0f、y軸方向にkPlayerCenterだけオフセットする
	m_attackCol->ColInit(player->m_pos, offset, 150.0f,
							ColliderType::Sphere, Tags::PlayerAttack, true,true);//攻撃の当たり判定を初期化する//最初は無効にしておく
	m_attackCol->SetIsActive(false);//最初は当たり判定を無効にしておく

}

void PlayerStateAttack::Update()
{
	auto player = m_owner.lock();
	if (!player) return;
	auto& input = Input::GetInstance();

	//攻撃中の移動処理
	AttackMoveMent();

	//コンボ予約の入力を取る//予約を取ったらもうここは通らないようにする
	AttackInputCheck();
	//回避
	if(input.IsTriggered("B") && player->IsAvoidable())
	{
		if (player->IsFloor())
		{
			AttackFinishProcess();//攻撃の段数を初期化するなどの処理
			player->ChangeState(std::make_shared<PlayerStateAvoid>(m_owner));
			return;
		}
	}

	//攻撃の進行率によってジャンプの入力を受け付けるかどうかを決める
	//ジャンプ
	if (input.IsTriggered("A"))
	{
		if (player->IsFloor())
		{
			AttackFinishProcess();//攻撃の段数を初期化するなどの処理
			player->ChangeState(std::make_shared<PlayerStateJump>(m_owner));
			return;
		}
		
	}
	float animRate = player->m_anim.GetAnimRate();
	//コンボに移行
	if (animRate >= 0.5f)
	{
		//通常攻撃からスキル攻撃に移行するとき//コンボではなく、スキル攻撃を初めて降ったというシステム
		if(m_isSkillAttackReserved)
		{
			m_isSkillAttackReserved = false;//スキル攻撃の予約を解除する
			AttackFinishProcess();//攻撃の段数を初期化するなどの処理
			//スキルゲージを減らす
			player->AddSkillGauge(-20);

			//スキル攻撃に移行する
			player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner, AttackType::SkillAttack));
			return;
		}

		//コンボ入力が予約されているとき
		if (m_isComboInputReserved)
		{
			//次のコンボに移行する
			StartCombo(m_nextComboIndex);//m_nextComboIndexもm_currentComboIndexも更新されている
			player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner,AttackType::None));
			return;
		}
	}

	//コンボ予約がなく、アニメーションが終わったとき//落下攻撃ではないとき
	int currentComboIndex = player->m_comboInfo.currentComboIndex;
	const ComboNode& node = player->m_comboChain[currentComboIndex];
	if (player->m_anim.GetAnimEndFlag() && node.moveSpeedY >= 0)
	{
		//攻撃が終了したら、Idle状態に遷移する//コンボインデックスを初期化
		AttackFinishProcess();
		if (player->IsFloor())
		{
			if (input.IsLeftStickInput())
			{
				//入力があればMove状態に遷移する
				player->ChangeState(std::make_shared<PlayerStateMove>(m_owner));
				return;
			}
			else
			{
				player->ChangeState(std::make_shared<PlayerStateIdle>(m_owner));
				return;
			}
		}
		else//空中は落下状態に移行
		{
			player->ChangeState(std::make_shared<PlayerStateFall>(m_owner));
			return;
		}
	}
	else if(node.moveSpeedY < 0)//落下攻撃の時は、地面と当たるまで//一旦地面に当たるまで
	{
		if (player->IsFloor())//地面と当たったとき
		{
			player->m_isGround = true;//地面にいる状態にする
			m_attackCol->SetIsActive(false);//攻撃の当たり判定を無効にする
			player->m_vel = Vector3(0, 0, 0);//突進が終わったら、速度を0にする
			//player->m_hitCol
			//攻撃判定を生成
			InpuctAttackSetUp();
			AttackFinishProcess();
			player->ChangeState(std::make_shared<PlayerStateIdle>(m_owner));
			return;
		}
	}
	
	//アニメーションの更新
	player->m_anim.Update();
}

void PlayerStateAttack::Exit()
{
	//攻撃の当たり判定を削除する//
	if (m_attackCol)
	{
		CollisionManager::GetInstance().ReleaseCollider(m_attackCol);//当たり判定を削除する
		m_attackCol->SetIsActive(false);
		m_attackCol->SetLifeTimeLimited();
		m_attackCol.reset();
	}
}

void PlayerStateAttack::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Attack");
	DrawFormatString(10, 30, GetColor(255, 255, 255), "ComboIndex:%d", m_owner.lock()->m_comboInfo.currentComboIndex);
	DrawFormatString(10, 50, GetColor(255, 255, 255), "m_attackCol Active:%d", m_attackCol->IsActive());
	//isHitの表示
	DrawFormatString(10, 70, GetColor(255, 255, 255), "isHit:%d", m_owner.lock()->m_comboInfo.isHit);
}

void PlayerStateAttack::AttackMoveMent()
{
	 auto player = m_owner.lock();
	if (!player) return;
	

	int currentComboIndex = player->m_comboInfo.currentComboIndex;
	const ComboNode& node = player->m_comboChain[currentComboIndex];
	float rate = player->m_anim.GetAnimRate();//アニメーションの進行率を取得

	//上下差がない攻撃とある攻撃で処理を分ける//moveSpeedYが0のときは、上下差がない攻撃とする
	if (node.moveSpeedY == 0.0f)
	{
		//攻撃判定//いったん
		if (rate > 0.2f && rate < 0.6f)
		{
			m_attackCol->SetIsActive(true);//攻撃の当たり判定を有効にする
		}
		else 
		{
			m_attackCol->SetIsActive(false);//攻撃の当たり判定を無効にする
		}


		//攻撃が当たったときは、動きを止める
		if (player->m_comboInfo.isHit)
		{
			player->m_vel = Vector3(0, 0, 0);//攻撃が当たったときは、速度を0にする
			return;
		}


		//コンボノードで設定された時間内だけ突進
		if (rate < node.moveFrame)
		{
			player->m_vel = player->m_targetVec * node.moveSpeedX;//攻撃の最初の数秒は前に突進する
		}
		else
		{
			player->m_vel = Vector3(0, 0, 0);//突進が終わったら、速度を0にする
		}
	}
	else//上下差あり
	{
		float timeScale = System::GetInstance().GetTimeScale();
		//重力
		player->m_accumulatedGravity += -Game::kGravity * timeScale;
		player->m_vel = m_InitVel + Vector3(0, player->m_accumulatedGravity,0);
		//player->m_vel += Vector3(0, -Game::kGravity, 0) * timeScale;

		//下方向は時間なし//上方向は時間制限あり
		if(node.moveSpeedY > 0)//上向き
		{
			//床から離れる
			player->SetIsFloor(false);
			if (player->m_vel.y <= 0)//速度が0になったら上昇終了
			{
				//player->m_vel = player->m_targetVec * node.moveSpeedX + Vector3(0, 0, 0);
				player->m_vel = Vector3(0, 0, 0);//終わったら、速度を0にする
				m_attackCol->SetIsActive(false);//攻撃の当たり判定を無効にする
			}
			//まだ上昇中
			else
			{
				//判定を有効
				m_attackCol->SetIsActive(true);//攻撃の当たり判定
			}
		}
		else//下向き//常に下方向の速度を与える
		{
			//判定を有効
			m_attackCol->SetIsActive(true);//攻撃の当たり判定
		}
		

	}
	
}

void PlayerStateAttack::DetermineAttackDirection()
{
	auto player = m_owner.lock();
	if (!player) return;

	auto& input = Input::GetInstance();
	auto& camera = player->m_camera;
	Vector3 attackDir = Vector3(0, 0, 0);

	//攻撃の方向を決める//カメラの向きと入力から、回避の方向を決める
	if (input.IsPressed("Up"))
	{
		attackDir += player->forward;
	}
	if (input.IsPressed("Down"))
	{
		attackDir += player->down;
	}
	if (input.IsPressed("Left"))
	{
		attackDir += player->left;
	}
	if (input.IsPressed("Right"))
	{
		attackDir += player->right;
	}
	//入力がないときは、playerの向いている方向に進む//あるときはその方向に進む//この処理に問題があるらしい
	if (attackDir.Magnitude() <= 0.0f)
	{
		player->m_targetVec = player->m_targetVec.Normalize();

		//player->m_targetVec = player->forward.Normalize();
	}
	else
	{
		player->m_targetVec = attackDir.Normalize();
	}
	
}

void PlayerStateAttack::AttackInputCheck()
{
	auto player = m_owner.lock();
	if (!player) return;
	auto& input = Input::GetInstance();
	//既に予約があるならリターン
	if (m_isComboInputReserved) return;

	//攻撃の入力を受け付けるか
	float animRate = player->m_anim.GetAnimRate();
	bool canInput = animRate >= kComboInputStart && animRate <= kComboInputEnd;//コンボ入力受付時間内かどうか//0.2から0.8の間
	if (!canInput)return;//コンボ入力受付時間外なら、ここで処理を終える

	//現在のコンボノードを取得
	int currentComboIndex = player->m_comboInfo.currentComboIndex;
	const ComboNode& currentNode = player->m_comboChain[currentComboIndex];
	//スキル攻撃かどうか
	bool isSkillAttack = currentNode.index == ComboIndex::SkillAttack1 ||
						 currentNode.index == ComboIndex::SkillAttack2 ||
				   	     currentNode.index == ComboIndex::SkillAttack3;
	bool isPlayerAir = !player->IsFloor();//空中にいるかどうか//空中にいるときは、空中攻撃に移行する
	bool WasSkillAirAttack = player->m_comboInfo.isAirSkillAttack;//空中でスキル攻撃をしたかどうか
	bool WasAirAttack = player->m_comboInfo.isAirAttack;//空中で攻撃をしたかどうか

	//スキル攻撃
	if (input.IsPressed("LB") && input.IsTriggered("X"))
	{
		//スキル攻撃ができるかどうか
		if (player->CanSkillAttack(false))
		{
			//通常攻撃ならば、攻撃を終了して、スキル攻撃に移行
			//スキル攻撃ならばコンボ攻撃に移行
			if (isSkillAttack)
			{
				//コンボ攻撃に移行する
				//弱攻撃ボタンでつながる次のコンボがあるか
				if (!currentNode.nextWeakAttack.empty())//空じゃなかったら
				{
					m_nextComboIndex = currentNode.nextWeakAttack[0];//次のコンボ番号をセットする//今回は1つしかないので、0番目をセットする
					m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
				}
			}
			//コンボ処理を終了して、スキル攻撃に移行する
			else
			{
				//空中でスキル攻撃を行っていたらスキル攻撃に移行しない
				if (isPlayerAir && WasSkillAirAttack)return;

				m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
				m_isSkillAttackReserved = true;//スキル攻撃の予約がされているフラグを立てる
			}
		}
	}
	//強攻撃
	else if (!input.IsPressed("LB") && input.IsTriggered("Y"))
	{
		//スキルアタックだったら弱攻撃につなぐ
		if (isSkillAttack)
		{
			//地上だったら弱攻撃1に移行する//空中だったら空中弱攻撃に移行する
			if (player->IsFloor())
			{
				m_nextComboIndex = ComboIndex::HeavyAttack1;//強攻撃1に移行する
				m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
			}
			else
			{
				m_nextComboIndex = ComboIndex::AirHeavyAttack1;//空中強攻撃に移行する
				m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
			}

		}

		if (isSkillAttack)return;
		if (!currentNode.nextHeavyAttack.empty())//空じゃなかったら
		{
			m_nextComboIndex = currentNode.nextHeavyAttack[0];//次のコンボ番号をセットする//今回は1つしかないので、0番目をセットする
			m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
		}
	}
	//弱攻撃
	else if (!input.IsPressed("LB") &&input.IsTriggered("X"))
	{
		//スキルアタックだったら弱攻撃につなぐ
		if (isSkillAttack)
		{
			//地上だったら弱攻撃1に移行する//空中だったら空中弱攻撃に移行する
			if (player->IsFloor())
			{
				m_nextComboIndex = ComboIndex::LightAttack1;//弱攻撃1に移行する
				m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
			}
			else
			{
				//初めての弱攻撃ならする
				if (!player->m_comboInfo.isAirAttack)
				{
					m_nextComboIndex = ComboIndex::AirAttack1;//空中弱攻撃に移行する
					m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
				}
			}

		}

		if (isSkillAttack)return;
		if (!currentNode.nextWeakAttack.empty())//空じゃなかったら
		{
			m_nextComboIndex = currentNode.nextWeakAttack[0];//次のコンボ番号をセットする//今回は1つしかないので、0番目をセットする
			m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
		}
		
	}
	//else if (!input.IsPressed("LB") && input.IsTriggered("Y"))
	//{
	//	//強攻撃ボタンでつながる次のコンボがあるか
	//	if (!currentNode.nextHeavyAttack.empty())//空じゃなかったら
	//	{
	//		m_nextComboIndex = currentNode.nextHeavyAttack[0];//次のコンボ番号をセットする//今回は1つしかないので、0番目をセットする
	//		m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
	//	}
	//}
}

void PlayerStateAttack::StartCombo(int comboIndex)
{
	auto player = m_owner.lock();
	if (!player) return;
	//範囲外だったら早期リターン
	if (comboIndex < 0 || comboIndex >= player->m_comboChain.size())return;
	//isHitをfalseにする
	player->m_comboInfo.isHit = false;
	//鴉状態を解除する
	player->m_isRaven = false;
	//現在のコンボの段数を更新
	player->m_comboInfo.currentComboIndex = comboIndex;
	m_isComboInputReserved = false;//コンボ入力の予約を解除する

	//Skill攻撃2,3だったらスキルゲージを減らす
	int currentComboIndex = player->m_comboInfo.currentComboIndex;	
	bool isSkillAttack2or3 = 
		currentComboIndex == ComboIndex::SkillAttack2 ||
		currentComboIndex == ComboIndex::SkillAttack3;

	if (isSkillAttack2or3)
	{
		player->AddSkillGauge(-20);
	}
}

void PlayerStateAttack::AttackFinishProcess()
{
	auto player = m_owner.lock();
	if (!player) return;
	player->m_comboInfo.currentComboIndex = ComboIndex::None;//攻撃していない状態に戻す
	player->m_comboInfo.isHit = false;//攻撃が当たったかどうか
	//y軸の速度を0に戻す
	player->m_vel.y = 0.0f;
	//鴉状態を解除する
	player->m_isRaven = false;
	
	//攻撃の当たり判定の開放
	//m_attackCol->SetIsActive(false);
	//m_attackCol->SetLifeTimeLimited();
	//m_attackCol.reset();
}

int PlayerStateAttack::SelectAnimInit()
{
	auto player = m_owner.lock();
	if (!player)return -1;

	//アニメーションの初期化
	int currentComboIndex = player->m_comboInfo.currentComboIndex;//現在のコンボインデックスを取得する
	if (currentComboIndex == ComboIndex::None)//コンボの段数が-1のときは、最初のコンボを再生する
	{

			
			if (m_attackType == AttackType::lightAttack)
			{
				if (player->IsFloor())
				{
					currentComboIndex = ComboIndex::LightAttack1;//弱攻撃の最初の段数を0に設定する
				}
				else
				{
					player->m_comboInfo.isAirAttack = true;//空中攻撃のフラグを立てる
					currentComboIndex = ComboIndex::AirAttack1;//空中攻撃1
				}
			}
			else if (m_attackType == AttackType::heavyAttack)
			{
				if (player->IsFloor())
				{
					currentComboIndex = ComboIndex::HeavyAttack1;//強攻撃の最初の段数を1に設定する//今回は、弱攻撃が0番目、強攻撃が1番目の段数から始まるようにする
				}
				else
				{
					currentComboIndex = ComboIndex::AirHeavyAttack1;//空中強攻撃1
				}
			}
			else if (m_attackType == AttackType::SkillAttack)
			{
				currentComboIndex = ComboIndex::SkillAttack1;
				if(!player->IsFloor())player->m_comboInfo.isAirSkillAttack = true;//空中攻撃のフラグを立てる
				
			}
			//スキルアタックだったら鴉状態にする
			bool isSkillAttack = currentComboIndex == ComboIndex::SkillAttack1 ||
								 currentComboIndex == ComboIndex::SkillAttack2 ||
								 currentComboIndex == ComboIndex::SkillAttack3;
			if (isSkillAttack)player->m_isRaven = true;//鴉状態にする
			else player->m_isRaven = false;//鴉状態を解除する
		//現在のコンボの段数を更新する
		player->m_comboInfo.currentComboIndex = currentComboIndex;
	}
	else
	{
		//コンボ攻撃で空中攻撃だった時、AirAttackをtrueにする
		if (currentComboIndex == ComboIndex::AirAttack1)
		{
						player->m_comboInfo.isAirAttack = true;
		}
		//スキルアタックだったら鴉状態にする
		bool isSkillAttack = currentComboIndex == ComboIndex::SkillAttack1 ||
			currentComboIndex == ComboIndex::SkillAttack2 ||
			currentComboIndex == ComboIndex::SkillAttack3;
		if (isSkillAttack)player->m_isRaven = true;//鴉状態にする
		else player->m_isRaven = false;//鴉状態を解除する

		//コンボの段数が-1でないときは、次のコンボを再生する
		//currentComboIndex = m_nextComboIndex;//次のコンボの段数を取得する
		//player->m_comboInfo.currentComboIndex = currentComboIndex;//現在のコンボの段数を更新する
	}
	return currentComboIndex;
}

void PlayerStateAttack::InpuctAttackSetUp()
{
	auto player = m_owner.lock();
	if (!player) return;
	const ComboNode& node = player->m_comboChain[player->m_comboInfo.currentComboIndex];
	float totalAnimFrame = player->m_anim.GetAnimTotalFrame(node.animName);
	//ドロップ攻撃の時は当たり判定を生成
	if (node.moveSpeedY < 0)
	{
		AttackData dropAttackData = {
			.attackPower = 0.0f,
			.knockBackPower = Vector3(20, 20, 0),
			.knockBackFrame = totalAnimFrame,
			.hitStopTime = 0.1f,
			.kAttackColOffset = 30.0f,
			.isKirimomi = true
		};
		//AttackColを生成
		auto m_attackColForDrop = std::make_shared<AttackCol>(m_owner, dropAttackData);
		m_attackColForDrop->ColInit(player->m_pos, Vector3(0, kPlayerCenter, 0), 150.0f,
			ColliderType::Sphere, Tags::PlayerAttack, true, true,10.0f);//攻撃の当たり判定を初期化する//最初は無効にしておく
		m_attackColForDrop->SetIsActive(true);//攻撃の当たり判定を有効にする
	}
}
