#include "EnemySwordman.h"
#include "../../../Math/Matrix4x4.h"
#include "../../Player/Base/Player.h"
#include "../../../Game.h"

namespace
{
	const std::string kAttackName = "mixamo.com";
	const std::string kIdleName = "Take 001";

	constexpr float kEnemyCenter = 100.0f;//敵の当たり判定の中心点までのy軸の距離

	constexpr float kEnemyMeleeAttackRange = 80.0f;//敵の近接攻撃の距離
	constexpr float kEnemyBackDistance = 600.0f;//敵が距離を取るときの距離


	constexpr float kEnemyIdleMaxTime = 60.0f;//敵がIdle状態でいる時間の最大値
	constexpr float kEnemyTargetUpdateTime = 30.0f;//敵がターゲットを更新する時間
	constexpr float kEnemyCautionMaxTime = 600.0f;//敵が警戒する時間の最大値
	constexpr float kEnemyAttackCoolTime = 60.0f;//敵の攻撃のクールタイム
	constexpr float kEnemyHitBackTime = 30.0f;//敵が攻撃を受けたときの吹き飛ばしの時間
	constexpr float kEnemyAirTime = 30.0f;//敵が空中にとどまるる時間
}

EnemySwordman::EnemySwordman(std::weak_ptr<Player> player) : EnemyBase(player)
{
	m_pos = Vector3(0, 0, 200);//初期位置
	m_hp = 500;//体力
	//一旦モデルはここ
	m_modelHandle = MV1LoadModel("data/Enemy/swordman.mv1");
	//モデルの初期位置を設定する
	Matrix4x4 rotY = Matrix4x4::MakeRotationY(0);
	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);
	Matrix4x4 mtx = trans * rotY;
	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));
	m_anim.Init(m_modelHandle, kAttackName, true);
}

EnemySwordman::~EnemySwordman()
{
	MV1DeleteModel(m_modelHandle);
}

void EnemySwordman::Init()
{
	//IDの取得
	SetID();
	//当たり判定の初期化
	ColInit(m_pos, Vector3(0, kEnemyCenter, 0), 50.0f, ColliderType::Sphere, Tags::Enemy, true);//中心点、半径、当たり判定のタイプ、タグ、当たり判定が有効かどうか
	//やられ判定の初期化
	InitHitCol(weak_from_this());
	m_hitCol->ColInit(m_pos, Vector3(0, kEnemyCenter, 0),80.0f, ColliderType::Sphere, Tags::EnemyHit, true,true);
}

void EnemySwordman::Update()
{
	auto player = m_player.lock();
	//敵が攻撃するかは現在、距離でやっているが、いずれ当たり判定でする
	
	//クールタイム
	//攻撃のクールタイムを減らす
	if (m_attackCoolTime > 0.0f)
	{
		m_attackCoolTime -= 1.0f;
	}
	//押し戻しの処理が続かないように消す//応急処置
	m_vel = Vector3(0, m_vel.y, 0);

	//Idle->ランダム回す仕組みを作る
	switch (m_state)
	{
	case EnemyState::Idle:
		//Playerを見る
		ToPlayerLook();

		m_idleTime += 1.0f;
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
		if (m_cautionTime++ > kEnemyCautionMaxTime)
		{
			m_cautionTime = 0.0f;
			ChangeState(EnemyState::Chase);
			break;
		}
		//定期的にプレイヤーの位置を更新する
		if (m_cautionTime > kEnemyCautionMaxTime)
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
			m_chasingTime += 1.0f;
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
		m_knockBackFrame++;
		
		switch (m_hitType)
		{
			case HitType::Air:
				//速度を指定
				m_vel = m_knockBackVel;
				//空中にいるときは、y軸の吹き飛ばしの力を減衰させる
				m_knockBackVel.y -= Game::kGravity;
				if (m_knockBackVel.y <= 0.0f)
				{
					m_knockBackVel.y = 0.0f;
					FinishHitProcess();
					//吹き飛ばしのベクトルが0になったらAirStayに戻す
					ChangeState(EnemyState::AirStay);
				}
				break;
			case HitType::Ground://上下差がないとき
				m_vel += m_knockBackVel;
				if (m_knockBackFrame > kEnemyHitBackTime)
				{
					
					FinishHitProcess();
					//現在地上か空中かで分岐
					if (m_pos.y <= 0.0f)
					{
						ChangeState(EnemyState::Idle);
					}
					else
					{
						ChangeState(EnemyState::Fall);
					}
				}
				break;
			case HitType::Drop:
				//速度を指定
				m_vel = m_knockBackVel;
				m_knockBackVel.y -= Game::kGravity;
				if (m_pos.y <= 0.0f)
				{
					m_pos.y = 0.0f;
					m_vel.y = 0.0f;
					m_knockBackVel.y = 0.0f;

					FinishHitProcess();
					//地面についたらChange
					ChangeState(EnemyState::Idle);
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
		m_vel.y -= Game::kGravity;//重力の処理
		if (m_pos.y <= 0.0f)
		{
			m_pos.y = 0.0f;
			m_vel.y = 0.0f;
			ChangeState(EnemyState::Idle);
		}
		break;
	default:
		break;

	}

	m_anim.Update();
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
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Enemy State: %s", stateString.c_str());
#endif
	MV1DrawModel(m_modelHandle);
}

void EnemySwordman::Attack()
{
}

void EnemySwordman::OnCollision(Collider& other)
{
	//当たっただけはなにもしない
}

void EnemySwordman::OnDamage(Collider& other, AttackData& data)
{
	//Playerの攻撃データをもとに被ダメ処理をする
	m_hp -= data.attackPower;
	//Enemy->Playerのベクトルに吹き飛ばす力を加える
	Vector3 pushBackVec = (other.GetPos() - m_pos).Normalize() * data.knockBackPower.x;

	//ここをknockBackVelにして、knockBackVelをだんだん減衰させる処理をする
	m_knockBackVel = pushBackVec;
	m_knockBackVel.y = data.knockBackPower.y;//Y軸の吹き飛ばしの力を加える
	DrawFormatString(500, 0, GetColor(255, 0, 0), "EnemySwordman: OnDamage");
	//stateをHItにする
	ChangeState(EnemyState::Hit);
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
		break;
	case EnemyState::Back:
		m_vel = Vector3(0, 0, 0);
		break;
	case EnemyState::Hit:
		
		break;
	case EnemyState::Fall:
		break;
	default:
		break;
	}
	//newStateに更新
	m_state = newState;
	switch (m_state)
	{
	case EnemyState::Idle:
		m_anim.ChangeAnim(kIdleName, true);
		break;
	case EnemyState::Chase:
		m_anim.ChangeAnim(kIdleName, true);
		m_targetPos = player->GetPos();
		//Playerを見る
		ToPlayerLook();
		break;
	case EnemyState::Caution:
		m_anim.ChangeAnim(kIdleName, true);
		m_targetPos = player->GetPos();
		//Playerを見る
		ToPlayerLook();
		break;
	case EnemyState::Attack:
		m_anim.ChangeAnim(kAttackName, false);
		//Playerを見る
		ToPlayerLook();
		break;
	case EnemyState::Back:
		m_anim.ChangeAnim(kIdleName, true);
		m_targetPos = player->GetPos();
		//Playerを見る
		ToPlayerLook();
		break;
	case EnemyState::Hit:
		if (m_knockBackVel.y > 0.0f)m_hitType = HitType::Air;
		else if(m_knockBackVel.y < 0.0f)m_hitType = HitType::Drop;
		else m_hitType = HitType::Ground;
		break;
	case EnemyState::AirStay:
		break;
	case EnemyState::Fall:
		break;
	default:
		break;
	}
}

