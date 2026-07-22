#include "EnemySwordman.h"
#include "../../../Math/Matrix4x4.h"
#include "../../Player/Base/Player.h"
#include "../../AttackCol.h"
#include "../../../Game.h"
#include "../System.h"

namespace
{
	const std::string kAttack = "Player|Attack";
	const std::string kIdle = "Player|Player|Idle";
	const std::string kHitName = "Player|hit";
	const std::string kRunName = "Player|Run";
	const std::string kStrafeRight = "Player|strafe_right";
	const std::string kStrafeLeft = "Player|strafe_left";
	const std::string kKirimomi = "Player|kirimomi";
	const std::string kKirimomi2 = "Player|kirimomi2";
	const std::string kBack = "Player|dodge_bac";
	const std::string kDie = "Player|Die";


	constexpr float kEnemyCenter = 100.0f;//敵の当たり判定の中心点までのy軸の距離

	constexpr float kEnemyMeleeAttackRange = 400.0f;//敵の近接攻撃の距離
	constexpr float kEnemyBackDistance = 600.0f;//敵が距離を取るときの距離

	constexpr float kEnemyIdleMaxTime = 60.0f;//敵がIdle状態でいる時間の最大値
	constexpr float kEnemyTargetUpdateTime = 30.0f;//敵がターゲットを更新する時間
	constexpr float kEnemyCautionMaxTime = 600.0f;//敵が警戒する時間の最大値
	constexpr float kEnemyAttackCoolTime = 60.0f;//敵の攻撃のクールタイム
	constexpr float kEnemyHitBackTime = 30.0f;//敵が攻撃を受けたときの吹き飛ばしの時間
	constexpr float kEnemyAirTime = 90.0f;//敵が空中にとどまるる時間
	constexpr float kEnemyDistance = 50.0f;
	constexpr float kToTargetPower = 3.0f;//プレイヤーの正面に行くようにknockBackする力
}

EnemySwordman::EnemySwordman(std::weak_ptr<Player> player, Vector3 pos, int modelHandle) : EnemyBase(player)
{
	m_pos = pos;//初期位置
	m_hp = 500;//体力
	//モデルのハンドルをセット
	m_modelHandle = modelHandle;
	//モデルの初期位置を設定する
	Matrix4x4 rotY = Matrix4x4::MakeRotationY(0);
	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);
	Matrix4x4 mtx = trans * rotY;
	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));
	m_anim.Init(m_modelHandle, kIdle, true);
	

}

EnemySwordman::~EnemySwordman()
{
	MV1DeleteModel(m_modelHandle);
}

void EnemySwordman::Init()
{
	//IDの取得
	SetID();
	//当たり判定の初期化j
	ColInit(m_pos, Vector3(0, kEnemyCenter, 0), 80.0f, ColliderType::Sphere, Tags::Enemy, true);//中心点、半径、当たり判定のタイプ、タグ、当たり判定が有効かどうか
	//やられ判定の初期化
	InitHitCol(GetWeakPtr());
	m_hitCol->ColInit(m_pos, Vector3(0, kEnemyCenter, 0),120.0f, ColliderType::Sphere, Tags::EnemyHit, true,true);
	m_hitCol->ResetID(GetId());
	//AttackColの生成
	m_attackData = {
		.attackPower = 10.0f,
		.knockBackPower = Vector3(0.0f, 0.0f, 0.0f),
		.knockBackFrame = 30.0f,
		.hitStopTime = 0.0f,
		.kAttackColOffset = 30.0f,
		.isKirimomi = false
	};
	m_attackCol = std::make_shared<AttackCol>(GetWeakPtr(), m_attackData);

	Vector3 offset = m_targetVec.Normalize() * m_attackData.kAttackColOffset + Vector3(0,kEnemyCenter,0);
	m_attackCol->ColInit(m_pos, offset, 100.0f, ColliderType::Sphere, Tags::EnemyAttack, false, true);
	m_attackCol->ResetID(GetId());

}


void EnemySwordman::Update()
{
	auto player = m_player.lock();
	//敵が攻撃するかは現在、距離でやっているが、いずれ当たり判定でする
	
	//クールタイム
	//攻撃のクールタイムを減らす
	if (m_attackCoolTime > 0.0f)
	{
		float timeScale = System::GetInstance().GetTimeScale();

		m_attackCoolTime -= 1.0f * timeScale * m_ownTimeScale;
	}
	//押し戻しの処理が続かないように消す//応急処置
	m_vel = Vector3(0, m_vel.y, 0);
	float timeScale = System::GetInstance().GetTimeScale();

	//Idle->ランダム回す仕組みを作る
	switch (m_state)
	{
	case EnemyState::Idle:
		//Playerを見る
		ToPlayerLook();
		m_idleTime += 1.0f * timeScale * m_ownTimeScale;
		//一定時間Idle状態でいる
		if (m_idleTime < kEnemyIdleMaxTime)break;

		//ランダムでChaseかCautionに遷移する
		if (CanMeleeAttack(kEnemyMeleeAttackRange))
		{
			ChangeState(EnemyState::Attack);
			break;
		}
		//ランダム
		if (rand() % 100 < 50)
		{
			ChangeState(EnemyState::Chase);
			break;
		}
		else if (rand() % 100 >= 50)
		{
			ChangeState(EnemyState::Caution);
			break;
		}
		break;
	case EnemyState::Caution:
		//Playerを見る
		ToPlayerLook();

		//一定時間様子を見る
		//その後、Chaseに移行
	
		m_cautionTime += 1.0f * timeScale * m_ownTimeScale;
		if (m_cautionTime > kEnemyCautionMaxTime)
		{
			m_cautionTime = 0.0f;
			ChangeState(EnemyState::Chase);
			break;
		}
		//定期的にプレイヤーの位置を更新する
		if (TickInterval(m_cautionUpdateTimer, kEnemyCautionMaxTime / 3))
		{
			m_targetPos = TargetPlayerPos();
		}
		CautionMove(m_targetPos, 400.0f);
		break;
	case EnemyState::Chase:
		//攻撃可能な距離に入ったら攻撃
		if (CanMeleeAttack(kEnemyMeleeAttackRange))
		{
			ChangeState(EnemyState::Attack);
			break;
		}
		//定期的にプレイヤーの位置を更新する
		if (!ChasePlayer(m_targetPos, kEnemyMeleeAttackRange))
		{
			m_chasingTime += 1.0f * timeScale * m_ownTimeScale;
			//更新
			if (m_chasingTime > kEnemyTargetUpdateTime)
			{
				m_chasingTime = 0.0f;
				m_targetPos = TargetPlayerPos();
				//Playerを見る
				ToPlayerLook();
			}
		}
		else
		{
			//いろいろやってもなぜか攻撃せず止まるので、距離関係なく攻撃させる
			ChangeState(EnemyState::Attack);
			break;
		}
		break;
	case EnemyState::Attack:
		Attack();
		if (m_anim.GetAnimEndFlag())
		{
			m_attackCoolTime = kEnemyAttackCoolTime;//攻撃のクールタイムをリセット
			ChangeState(EnemyState::Back);
			break;
		}
		break;
	case EnemyState::Back:
		//敵の挙動改良案
		//バックステップの距離をランダムにする
		//確率でバックステップではなくCautionにする

		//一定距離離れたらIdle
		if (BackMove(m_targetPos, kEnemyBackDistance))
		{
			ChangeState(EnemyState::Idle);
			m_idleTime = 0.0f;
			break;
		}
		break;
	case EnemyState::Hit:
		//m_knockBackVelで保存したベクトルをm_velに追加
		//y軸があるときとないときで処理を変える
		m_knockBackFrame += 1.0f * timeScale * m_ownTimeScale;
		
		switch (m_hitType)
		{
			case HitType::Air:
				//空中にいるときは、y軸の吹き飛ばしの力を減衰させる
				m_accumulatedGravity += Game::kGravity * timeScale * m_ownTimeScale;
				//速度を指定
				m_vel = m_knockBackVel + Vector3(0, -m_accumulatedGravity, 0);
				//m_knockBackVel.y -= Game::kGravity
				if (m_vel.y <= 0.0f)
				{
					m_knockBackVel.y = 0.0f;
					m_vel.y = 0.0f;
					m_accumulatedGravity = 0.0f;//重力の累積値をリセット
					FinishHitProcess();
					//吹き飛ばしのベクトルが0になったらAirStayに戻す
					ChangeState(EnemyState::AirStay);
				}
				break;
			case HitType::Ground://上下差がないとき
			{//バイパス防止(switch文の中で初期化するのはコンパイルエラーになるので、波括弧でくくって明示する)
				//PlayerとEnemyのベクトル方向にとばす
				auto player = m_player.lock();
				if (!player)return;
				//敵を移動させる
				if (m_knockBackFrame <= m_attackData.knockBackFrame)
				{
					//Enemy->Playerのベクトルに吹き飛ばす力を加える//プレイヤーの正面に行くようにknockBackする//いずれkirimomi吹っ飛びの時の処理と分ける
					Vector3 front = player->GetTargetVec();
					Vector3 pos = player->GetPos() + player->GetVel();
					Vector3 TargetPos = pos + front * kEnemyDistance;
					Vector3 toTarget = (TargetPos - m_pos).Normalize() * kToTargetPower;

					Vector3 knockBackDir = (m_pos - player->GetPos()).Normalize();
					knockBackDir.y = 0.0f;//y軸の吹き飛ばしはなし
					knockBackDir += toTarget;
					m_vel += knockBackDir * m_attackData.knockBackPower.x;
				}
				
				if (m_knockBackFrame > kEnemyHitBackTime)//本来はplayerの攻撃終了タイミングを読み取って、そこから変わる
				{

					FinishHitProcess();
					//現在地上か空中かで分岐//床についているかどうかで分岐
					if (IsFloor())
					{
						ChangeState(EnemyState::Idle);
					}
					else
					{
						ChangeState(EnemyState::AirStay);
					}
				}
			}
				break;
			case HitType::Drop:

				m_accumulatedGravity += Game::kGravity * timeScale * m_ownTimeScale;
				//速度を指定
				m_vel = m_knockBackVel + Vector3(0, -m_accumulatedGravity, 0);

				//速度を指定
				//m_knockBackVel.y += -Game::kGravity;
				//m_vel = m_knockBackVel;
				if (IsFloor())
				{
					if(m_isDieOut)
					{
						m_isDead = true;
						ChangeState(EnemyState::Dead);
						return;
					}


					//m_pos.y = 0.0f;
					m_vel.y = 0.0f;
					m_accumulatedGravity = 0.0f;//重力の累積値をリセット
					m_knockBackVel.y = 0.0f;

					FinishHitProcess();
					//地面についたらChange//knockDown状態とか作ったっていい
					ChangeState(EnemyState::KnockDown);
				}
				break;
			default : 
				break;
		}
		break;
	case EnemyState::AirStay:
		m_airCount += 1.0f;
		if (m_airCount > kEnemyAirTime)
		{
			m_airCount = 0.0f;
			FinishHitProcess();
			ChangeState(EnemyState::Fall);
		}
		break;
	case EnemyState::Fall:
		//落下処理
		//m_vel.y -= Game::kGravity;//重力の処理
		m_accumulatedGravity += Game::kGravity * timeScale * m_ownTimeScale;
		m_vel.y -= m_accumulatedGravity;
		if (IsFloor())
		{
			//m_pos.y = 0.0f;
			m_vel.y = 0.0f;
			m_accumulatedGravity = 0.0f;//重力の累積値をリセット
			ChangeState(EnemyState::Idle);
		}
		break;
	case EnemyState::Dead:
		if (m_anim.GetAnimEndFlag())
		{
			m_isDead = true;
		}
		break;
	case EnemyState::KnockDown:
		//吹き飛んだ後のダウン時間
		m_knockBackDownFrame += 1.0f * timeScale * m_ownTimeScale;
		if (m_knockBackDownFrame > 55)
		{
			m_knockBackDownFrame = 0.0f;
			ChangeState(EnemyState::Idle);
		}
		break;
	default:
		break;

	}
	//いったんここで、敵の吸着の仕様を作る



	m_anim.Update(m_ownTimeScale);


	//いったん対策で敵のy座標が-300ぐらいで殺す
	if(m_pos.y < -300.0f)
	{
		m_isDead = true;
	}

	//m_pos += m_vel;//速度を座標に加算する//移動する
	//回転と座標の更新
	//UpdateAngleAndPos();


	/*Matrix4x4 rotY = Matrix4x4::MakeRotationY(0);
	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);
	Matrix4x4 mtx = trans * rotY;
	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));*/
}

void EnemySwordman::Draw()
{
#ifdef _DEBUG
	//敵のStateを表示する
	std::string stateString = GetEnemyStateString(m_state);
	std::string hitTypeString = GetHitString();

	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Enemy State: %s", stateString.c_str());
	DrawFormatString(Game::kScreenWidth - 600, 30, GetColor(255, 255, 255), "Enemy HitType: %s",hitTypeString.c_str() );
#endif
	if (!m_isDead)
	{
		//まだエネミーマネージャーが管理していないので、ここで描画だけ消す//見た目用
		MV1DrawModel(m_modelHandle);
	}
}

std::string EnemySwordman::GetHitString()
{
	std::string hitTypeString;
	switch (m_hitType)
	{
	case HitType::None:
		hitTypeString = "None";
		break;
	case HitType::Air:
		hitTypeString = "Air";
		break;
	case HitType::Ground:
		hitTypeString = "Ground";
		break;
	case HitType::Drop:
		hitTypeString = "Drop";
		break;
	default:
		break;
	}
	return hitTypeString;
}

void EnemySwordman::Attack()
{
	float rate = m_anim.GetAnimRate();
	Vector3 forward = m_targetPos - m_pos;
	forward.y = 0.0f;
	forward = forward.Normalize();
	//移動距離
	if (rate <= 0.3)
	{
		m_vel = forward * 10.0f;
	}
	else if(rate > 0.3 && rate <= 0.5)
	{
		m_vel = forward * 16.0f;
		m_attackCol->SetIsActive(true);
	}

	else
	{
		m_vel = Vector3(0, 0, 0);
	}
}

void EnemySwordman::OnCollision(Collider& other)
{
	//当たっただけはなにもしない
}

void EnemySwordman::OnDamage(Collider& other, AttackData& data)
{
	auto player = m_player.lock();
	if (!player)return;

	//データの保存
	m_attackData = data;
	////もし、ダメージが0以下なら無視せず、ヒットステートに更新する
	//if(m_attackData.attackPower <= 0.0f)
	//{
	//	ChangeState(EnemyState::Hit);
	//	return;
	//}

	//死亡していたら処理しない
	if (m_isDead)return;
	//死亡吹っ飛び中は処理しない
	if (m_isDieOut)return;
	//Playerの攻撃データをもとに被ダメ処理をする
	m_hp -= static_cast<int>(data.attackPower);
	if (m_hp <= 0)
	{
		//空中じゃ死なない
		if (!IsFloor())
		{
			m_hp = 1;
		}
		else
		{
			m_hp = 0;
			m_isLifeZero = true;
			//当たり判定を解除する
			Terminate();

			//キリモミ吹っ飛びの時は、途中で死ぬ
			if (m_attackData.isKirimomi)
			{
				m_isDieOut = true;
			}
			//死亡アニメーションに移行
			else
			{
				ChangeState(EnemyState::Dead);
				return;
			}
		}
	}
	//Enemy->Playerのベクトルに吹き飛ばす力を加える//プレイヤーの正面に行くようにknockBackする//いずれkirimomi吹っ飛びの時の処理と分ける
	Vector3 front = player->GetTargetVec();
	Vector3 pos = player->GetPos() + player->GetVel();
	Vector3 TargetPos = pos + front * kEnemyDistance;
	Vector3 toTarget = (TargetPos - m_pos).Normalize() * kToTargetPower;//プレイヤーの正面に行くようにknockBackする力//吸着

	Vector3 pushBackVec = (m_pos - other.GetPos()).Normalize() * data.knockBackPower.x;
	pushBackVec += toTarget;
	//nockbackテスト用
	if (m_attackData.isKirimomi)
	{
		// pushBackVec += (m_pos - other.GetPos()).Normalize() * 15.0f;
	}

	//ここをknockBackVelにして、knockBackVelをだんだん減衰させる処理をする
	m_knockBackVel = pushBackVec;
	m_knockBackVel.y = data.knockBackPower.y;//Y軸の吹き飛ばしの力を加える
	DrawFormatString(500, 0, GetColor(255, 0, 0), "EnemySwordman: OnDamage");
	//stateをHItにする
	ChangeState(EnemyState::Hit);
}

void EnemySwordman::Terminate()
{
	if (m_attackCol)
	{
		CollisionManager::GetInstance().ReleaseCollider(m_attackCol);
	}
	if(m_hitCol)
	{
		CollisionManager::GetInstance().ReleaseCollider(m_hitCol);
	}
}

void EnemySwordman::ChangeState(EnemyState newState)
{
	auto player = m_player.lock();
	if (!player)return;

	//現在の状態を終わらせる//追いかけ、警戒、バクステは座標をここでセットする
	switch (m_state)
	{
	case EnemyState::Idle:
		break;
	case EnemyState::Chase:
		m_vel = Vector3(0, 0, 0);
		break;
	case EnemyState::Caution:
		m_vel = Vector3(0, 0, 0);
		break;
	case EnemyState::Attack:
		m_vel = Vector3(0, 0, 0);
		m_attackCol->SetIsActive(false);
		m_attackCol->ClearHitIds();
		break;
	case EnemyState::Back:
		m_vel = Vector3(0, 0, 0);
		break;
	case EnemyState::Hit:
		if (newState == EnemyState::Hit)
		{
			m_knockBackFrame = 0;
			m_hitType = HitType::None;
		}
		//ここで速度を消しちゃうと次Hitの時、Ondamageで入れたm_knockBackVelが反映されない
		else m_vel = Vector3(0, 0, 0);
		break;
	case EnemyState::Fall:
		break;
	case EnemyState::Dead:
		break;
	case EnemyState::KnockDown:
		break;
	default:
		break;
	}
	//newStateに更新
	m_state = newState;
	switch (newState)
	{
	case EnemyState::Idle:
		m_anim.ChangeAnim(kIdle, true);
		m_idleTime = 0.0f;
		break;
	case EnemyState::Chase:
		m_anim.ChangeAnim(kRunName, true,0.8f);
		m_targetPos = player->GetPos();
		//Playerを見る
		ToPlayerLook();
		break;
	case EnemyState::Caution:
		//右と左でアニメーションを変える
		m_anim.ChangeAnim(kStrafeRight, true, 0.4f);
		m_targetPos = player->GetPos();
		//Playerを見る
		ToPlayerLook();
		break;
	case EnemyState::Attack:
		m_anim.ChangeAnim(kAttack, false, 0.9f);
		//Playerを見る
		ToPlayerLook();
		break;
	case EnemyState::Back:
		m_anim.ChangeAnim(kBack, false,0.5f);
		m_targetPos = player->GetPos();
		//Playerを見る
		ToPlayerLook();
		break;
	case EnemyState::Hit:
		m_anim.ChangeAnim(kHitName, false);
		if (m_knockBackVel.y > 0.0f)
		{
			m_hitType = HitType::Air;
			SetIsFloor(false);//空中にいるので、床にいないことにする
		}
		else if(m_knockBackVel.y < 0.0f)m_hitType = HitType::Drop;
		else m_hitType = HitType::Ground;
		if (m_attackData.isKirimomi)
		{
			m_anim.ChangeAnim(kKirimomi, false,0.8f);
			m_hitType = HitType::Drop;//吹き飛ぶときはDropにする
		}
		break;
	case EnemyState::AirStay:
		break;
	case EnemyState::Fall:
		break;
	case EnemyState::Dead:
		//死亡アニメーションを流す
		m_anim.ChangeAnim(kDie, false, 0.5f);
		break;
	case EnemyState::KnockDown:
		//knockDownフレームのリセット
		m_knockBackDownFrame = 0.0f;
		break;
	default:
		break;
	}
	//重力の累積値をリセットする
	m_accumulatedGravity = 0.0f;
}

bool EnemySwordman::TickInterval(float& timer, float interval)
{
	float timeScale = System::GetInstance().GetTimeScale();

	timer += timeScale * m_ownTimeScale;
	if (timer >= interval)
	{
		timer = 0.0f;
		return true;
	}
	return false;
}

