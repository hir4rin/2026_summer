#include "PlayerStateAttack.h"
#include "Player.h"
#include "../../../Game.h"
#include "../../../Input.h"
#include "../../AttackCol.h"


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
	//アニメーションの初期化
	int currentComboIndex = player->m_comboInfo.currentComboIndex;//現在のコンボインデックスを取得する
	if(currentComboIndex == ComboIndex::None)//コンボの段数が-1のときは、最初のコンボを再生する
	{
		if(m_attackType == AttackType::lightAttack)
		{
			if (player->IsFloor())currentComboIndex = ComboIndex::LightAttack1;//弱攻撃の最初の段数を0に設定する
			else currentComboIndex = ComboIndex::AirAttack1;//空中攻撃1
		}
		else if(m_attackType == AttackType::heavyAttack)
		{
			if (player->IsFloor())currentComboIndex = ComboIndex::HeavyAttack1;//強攻撃の最初の段数を1に設定する//今回は、弱攻撃が0番目、強攻撃が1番目の段数から始まるようにする
			else currentComboIndex = ComboIndex::AirHeavyAttack1;//空中強攻撃1
		}
		//現在のコンボの段数を更新する
		player->m_comboInfo.currentComboIndex = currentComboIndex;
	}
	else
	{
		//コンボの段数が-1でないときは、次のコンボを再生する
		//currentComboIndex = m_nextComboIndex;//次のコンボの段数を取得する
		//player->m_comboInfo.currentComboIndex = currentComboIndex;//現在のコンボの段数を更新する
	}

	const ComboNode& node = player->m_comboChain[currentComboIndex];
	player->m_anim.ChangeAnim(node.animName, false,1.0f);


	//上下差がある攻撃の時はここで初速を与える
	if (node.moveSpeedY != 0)
	{
		player->m_vel = player->m_targetVec * node.moveSpeedX + Vector3(0, node.moveSpeedY, 0);
		if (node.moveSpeedY > 0)
		{
			player->m_isGround = false;//ジャンプ状態にする
		}
	}
	float totalAnimFrame = player->m_anim.GetAnimTotalFrame(node.animName);
	//ここでColliderを生成する//あとhitstopとkAttackColOffset
	player->m_attackData = {
	.attackPower = node.attackPower,
	//.knockBackPower = Vector3(node.knockBackXZ, node.knockBackY,0),
	.knockBackPower = Vector3(0.0f,node.knockBackY,0.0f),//吹き飛ばない攻撃にする
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

	//コンボ予約の入力を取る
	AttackInputCheck();
	//回避
	if(input.IsTriggered("B") && player->IsAvoidable())
	{
		AttackFinishProcess();//攻撃の段数を初期化するなどの処理
		player->ChangeState(std::make_shared<PlayerStateAvoid>(m_owner));
		return;
	}

	//攻撃の進行率によってジャンプの入力を受け付けるかどうかを決める
	//ジャンプ
	if (input.IsTriggered("A"))
	{
		AttackFinishProcess();//攻撃の段数を初期化するなどの処理
		player->ChangeState(std::make_shared<PlayerStateJump>(m_owner));
		return;
	}
	float animRate = player->m_anim.GetAnimRate();
	//コンボに移行
	if (animRate >= 0.5f)
	{
		//コンボ入力が予約されているとき
		if (m_isComboInputReserved)
		{
			//次のコンボに移行する
			StartCombo(m_nextComboIndex);//m_nextComboIndexも更新されている
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
			//player->m_pos.y = 0.0f;//地面に埋まらないようにする
			player->m_vel = Vector3(0, 0, 0);//突進が終わったら、速度を0にする
			//player->m_hitCol
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
	
}

void PlayerStateAttack::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Attack");
	DrawFormatString(10, 30, GetColor(255, 255, 255), "ComboIndex:%d", m_owner.lock()->m_comboInfo.currentComboIndex);
	DrawFormatString(10, 50, GetColor(255, 255, 255), "m_attackCol Active:%d", m_attackCol->IsActive());
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
		//上下差ある攻撃は常に判定を有効
		m_attackCol->SetIsActive(true);//攻撃の当たり判定を無効にする
		//重力
		player->m_vel += Vector3(0, -Game::kGravity, 0);

		//下方向は時間なし//上方向は時間制限あり
		if(node.moveSpeedY > 0)//上向き
		{
			//床から離れる
			player->SetIsFloor(false);
			if (player->m_vel.y <= 0)//速度が0になったら上昇終了
			{
				//player->m_vel = player->m_targetVec * node.moveSpeedX + Vector3(0, 0, 0);
				player->m_vel = Vector3(0, 0, 0);//終わったら、速度を0にする
			}
		}
		else//下向き//常に下方向の速度を与える
		{
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

	//攻撃の入力を受け付けるか
	float animRate = player->m_anim.GetAnimRate();
	bool canInput = animRate >= kComboInputStart && animRate <= kComboInputEnd;//コンボ入力受付時間内かどうか//0.2から0.8の間
	if (!canInput)return;//コンボ入力受付時間外なら、ここで処理を終える

	//現在のコンボノードを取得
	int currentComboIndex = player->m_comboInfo.currentComboIndex;
	const ComboNode& currentNode = player->m_comboChain[currentComboIndex];

	if (input.IsTriggered("X"))
	{
		//弱攻撃ボタンでつながる次のコンボがあるか
		if (!currentNode.nextWeakAttack.empty())//空じゃなかったら
		{
			m_nextComboIndex = currentNode.nextWeakAttack[0];//次のコンボ番号をセットする//今回は1つしかないので、0番目をセットする
			m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
		}
	}
	else if (input.IsTriggered("Y"))
	{
		//強攻撃ボタンでつながる次のコンボがあるか
		if (!currentNode.nextHeavyAttack.empty())//空じゃなかったら
		{
			m_nextComboIndex = currentNode.nextHeavyAttack[0];//次のコンボ番号をセットする//今回は1つしかないので、0番目をセットする
			m_isComboInputReserved = true;//コンボ入力が予約されているフラグを立てる
		}
	}
}

void PlayerStateAttack::StartCombo(int comboIndex)
{
	auto player = m_owner.lock();
	if (!player) return;
	//isHitをfalseにする
	player->m_comboInfo.isHit = false;
	//範囲外だったら早期リターン
	if (comboIndex < 0 || comboIndex >= player->m_comboChain.size())return;

	//現在のコンボの段数を更新
	player->m_comboInfo.currentComboIndex = comboIndex;
	m_isComboInputReserved = false;//コンボ入力の予約を解除する

}

void PlayerStateAttack::AttackFinishProcess()
{
	auto player = m_owner.lock();
	if (!player) return;
	player->m_comboInfo.currentComboIndex = ComboIndex::None;//攻撃していない状態に戻す
	player->m_comboInfo.isHit = false;//攻撃が当たったかどうか
	//y軸の速度を0に戻す
	player->m_vel.y = 0.0f;
	//攻撃の当たり判定の開放
	m_attackCol.reset();
}