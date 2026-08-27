#include "BossEnemy.h"
#include "../../../Math/Matrix4x4.h"
#include "../../Player/Base/Player.h"
#include "../../AttackCol.h"
#include "../../../Game.h"
#include "../System.h"
#include "EffekseerForDXLib.h"
#include "../../../DataLoader/DataManager.h"

namespace
{
	const std::string kIdle = "Player|Player|Idle";

	constexpr float kEnemyCenter = 100.0f;//敵の当たり判定の中心点までのy軸の距離
	constexpr float kEnemyEfOffset = 130.0f;//敵のエフェクトの中心点までのy軸の距離

	constexpr float kUltDamagePower = 500.0f;//必殺技の攻撃力

	constexpr int kEnemyInitialHp = 5000;//敵の初期HP
	constexpr float kEnemyColliderRadius = 80.0f;//本体コライダーの半径
	constexpr float kEnemyHitColliderRadius = 120.0f;//やられ判定コライダーの半径

	constexpr float kEnemyAttackPower = 10.0f;//初期攻撃データの攻撃力
	constexpr float kEnemyAttackKnockBackFrame = 30.0f;//初期攻撃データのノックバック時間
	constexpr float kEnemyAttackColOffset = 30.0f;//初期攻撃データの攻撃判定オフセット
	constexpr float kEnemyAttackColliderRadius = 100.0f;//攻撃コライダーの半径

	constexpr float kEnemyDeathYThreshold = -300.0f;//敵を強制的に死亡させるY座標のしきい値

	constexpr float kEnemyDistance = 50.0f;
	constexpr float kToTargetPower = 3.0f;//プレイヤーの正面に行くようにknockBackする力

	constexpr float kAttackMoveStartRate = 0.3f;//攻撃モーション:移動を開始するrate
	constexpr float kAttackColActivateRate = 0.5f;//攻撃モーション:攻撃判定を有効にするrateの上限
	constexpr float kAttackMoveSpeed1 = 10.0f;//攻撃モーション前半の移動速度
	constexpr float kAttackMoveSpeed2 = 16.0f;//攻撃モーション後半の移動速度
}

BossEnemy::BossEnemy(std::weak_ptr<Player> player, Vector3 pos, int modelHandle) : EnemyBase(player)
{
	//アニメーションの名前のマップの初期化
	const auto& animData = DataManager::GetInstance().GetBossAnimData();
	m_animNames = animData.animNames;

	m_pos = pos;//初期位置
	m_hp = kEnemyInitialHp;//体力
	//モデルのハンドルをセット
	m_modelHandle = modelHandle;
	//モデルの初期位置を設定する
	Matrix4x4 rotY = Matrix4x4::MakeRotationY(0);
	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);
	//MATRIX scale = MGetScale(VGet(2.0f, 2.0f, 2.0f));
	Matrix4x4 mtx = trans * rotY;
	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));
	MV1SetScale(m_modelHandle, VGet(2.0f, 2.0f, 2.0f));
	m_anim.Init(m_modelHandle, kIdle, true);

	m_hitEfHandle = System::GetInstance().GetHandle(AsyncData::EnemyHitEffect);
}

BossEnemy::~BossEnemy()
{
	MV1DeleteModel(m_modelHandle);
}

void BossEnemy::Init()
{
	//IDの取得
	SetID();
	//当たり判定の初期化//中心点、半径、当たり判定のタイプ、タグ、当たり判定が有効かどうか
	ColInit(m_pos, Vector3(0, kEnemyCenter, 0), kEnemyColliderRadius, ColliderType::Sphere, Tags::Boss, true);
	//やられ判定の初期化
	InitHitCol(GetWeakPtr());
	m_hitCol->ColInit(m_pos, Vector3(0, kEnemyCenter, 0), kEnemyHitColliderRadius, ColliderType::Sphere, Tags::EnemyHit, true, true);
	m_hitCol->ResetID(GetId());
	//AttackColの生成
	m_attackData = {
		.attackPower = kEnemyAttackPower,
		.knockBackPower = Vector3(0.0f, 0.0f, 0.0f),
		.knockBackFrame = kEnemyAttackKnockBackFrame,
		.hitStopTime = 0.0f,
		.kAttackColOffset = kEnemyAttackColOffset,
		.isKirimomi = false
	};
	m_attackCol = std::make_shared<AttackCol>(GetWeakPtr(), m_attackData);

	Vector3 offset = m_targetVec.Normalize() * m_attackData.kAttackColOffset + Vector3(0, kEnemyCenter, 0);
	m_attackCol->ColInit(m_pos, offset, kEnemyAttackColliderRadius, ColliderType::Sphere, Tags::EnemyAttack, false, true);
	m_attackCol->ResetID(GetId());

	//初期状態をIdleにする
	ChangeState(std::make_shared<BossStateIdle>(GetWeakPtr()));
}

void BossEnemy::Update()
{
	//敵が攻撃するかは現在、距離でやっているが、いずれ当たり判定でする

	//クールタイム
	//攻撃のクールタイムを減らす
	if (m_attackCoolTime > 0.0f)
	{
		float timeScale = System::GetInstance().GetTimeScale();

		m_attackCoolTime -= 1.0f * timeScale * m_ownTimeScale;
	}
	//当たり判定の更新
	for (auto& col : m_attackCols)
	{
		col->Update();
	}


	//Effectの位置の更新
	if (m_hitEfPlayingHandle != -1)
	{
		SetPosPlayingEffekseer3DEffect(m_hitEfPlayingHandle, m_pos.x, m_pos.y + kEnemyEfOffset, m_pos.z);
	}
	//Effectの再生が終わったらハンドルをリセット
	if (IsEffekseer3DEffectPlaying(m_hitEfPlayingHandle) == -1 && m_hitEfPlayingHandle != -1)
	{
		m_hitEfPlayingHandle = -1;
	}

	//押し戻しの処理が続かないように消す//応急処置
	m_vel = Vector3(0, m_vel.y, 0);

	//状態の更新//挙動はBossState派生クラス側に持たせている
	if (m_currentState)
	{
		m_currentState->Update();
	}

	m_anim.Update(m_ownTimeScale);

	//いったん対策で敵のy座標が-300ぐらいで殺す
	if (m_pos.y < kEnemyDeathYThreshold)
	{
		m_isDead = true;
	}
}

void BossEnemy::Draw()
{
#ifdef _DEBUG
	//敵のStateを表示する//表示内容は各BossStateのDebugDrawに持たせている
	if (m_currentState)
	{
		m_currentState->DebugDraw();
	}
#endif
	if (!m_isDead)
	{
		//まだエネミーマネージャーが管理していないので、ここで描画だけ消す//見た目用
		MV1DrawModel(m_modelHandle);
	}


	for (auto& col : m_attackCols)
	{
		//col->DebugDraw();
	}

}

void BossEnemy::Attack()
{

}

void BossEnemy::OnCollision(Collider& other)
{
	//当たっただけはなにもしない
}

void BossEnemy::OnDamage(Collider& other, AttackData& data)
{
	//注意点
	//bossはあまり、被弾モーションにならない
	// その代わり、モーションが遅いのと、スタン時間がある
	// 体力はめっちゃ多い
	//　
	//

	auto player = m_player.lock();
	if (!player)return;

	//データの保存
	m_attackData = data;

	//死亡していたら処理しない
	if (m_isDead)return;
	//死亡吹っ飛び中は処理しない
	if (m_isDieOut)return;
	//Playerの攻撃データをもとに被ダメ処理をする
	m_hp -= static_cast<int>(data.attackPower);
	m_stunStack += static_cast<int>(data.attackPower);
	if (m_stunStack >= 100.0f)
	{
		//スタン状態に移行
	}

	//ダメージがあるなら、ヒットエフェクトを再生する//必殺技の時は、ヒットエフェクトをスローのものにする
	if (static_cast<int>(data.attackPower) > 0)
	{
		//必殺技
		if (data.attackPower >= kUltDamagePower)
		{
			//m_hitEfPlayingHandle = PlayEffekseer3DEffectSlow(m_hitEfHandle, 0.5f);
		}
		//その他
		else
		{
			m_hitEfPlayingHandle = PlayEffekseer3DEffect(m_hitEfHandle);
			SetPosPlayingEffekseer3DEffect(m_hitEfPlayingHandle, m_pos.x, m_pos.y + kEnemyEfOffset, m_pos.z);
		}
	}

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
				ChangeState(std::make_shared<BossStateDead>(GetWeakPtr()));
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

	//ここをknockBackVelにして、knockBackVelをだんだん減衰させる処理をする
	m_knockBackVel = pushBackVec;
	m_knockBackVel.y = data.knockBackPower.y;//Y軸の吹き飛ばしの力を加える
	DrawFormatString(500, 0, GetColor(255, 0, 0), "BossEnemy: OnDamage");
	//stateをHItにする
	ChangeState(std::make_shared<BossStateHit>(GetWeakPtr()));
}

void BossEnemy::Terminate()
{
	if (m_attackCol)
	{
		CollisionManager::GetInstance().ReleaseCollider(m_attackCol);
	}
	if (m_hitCol)
	{
		CollisionManager::GetInstance().ReleaseCollider(m_hitCol);
	}
}

void BossEnemy::RestartDeadState()
{
	//ChangeState自体は同じState同士でもExit→Enterをやり直す作りなので、
	//すでにDead中でもこれで死亡アニメーションを最初から再生し直せる
	ChangeState(std::make_shared<BossStateDead>(GetWeakPtr()));
}

void BossEnemy::ChangeState(EnemyState newState)
{
	//EnemyBaseのインターフェース用//外部からenumで状態を指定された場合にBossStateへ変換する
	switch (newState)
	{
	case EnemyState::Idle:
		ChangeState(std::make_shared<BossStateIdle>(GetWeakPtr()));
		break;
	case EnemyState::Caution:
		ChangeState(std::make_shared<BossStateCaution>(GetWeakPtr()));
		break;
	case EnemyState::Chase:
		ChangeState(std::make_shared<BossStateChase>(GetWeakPtr()));
		break;
	case EnemyState::Attack:
		ChangeState(std::make_shared<BossStateAttack>(GetWeakPtr()));
		break;
	case EnemyState::Back:
		ChangeState(std::make_shared<BossStateBack>(GetWeakPtr()));
		break;
	case EnemyState::Hit:
		ChangeState(std::make_shared<BossStateHit>(GetWeakPtr()));
		break;
	case EnemyState::Dead:
		ChangeState(std::make_shared<BossStateDead>(GetWeakPtr()));
		break;
	default:
		break;
	}
}

void BossEnemy::ChangeState(std::shared_ptr<BossState> newState)
{
	//playerがいなければ状態遷移しない
	auto player = m_player.lock();
	if (!player)return;

	//現在の状態から抜ける
	if (m_currentState)
	{
		m_currentState->Exit();
	}
	//newStateに更新
	m_currentState = newState;
	//newStateの初期化
	if (m_currentState)
	{
		m_currentState->Enter();
	}
	//重力の累積値をリセットする//状態遷移のたびに必ず行う
	m_accumulatedGravity = 0.0f;
}

bool BossEnemy::TickInterval(float& timer, float interval)
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

void BossEnemy::ApplyPos()
{
	//モデルの座標を更新する
	//CharacterBase::ApplyPos();

	m_pos += m_vel;

	float targetAngle = 0.0f;//目標の角度
	if (m_targetVec.Magnitude() > 0.0f)//最初の入力されないとき以外、ここを通り、モデルの向きを変える
	{
		//モデルの移動方向にモデルの方向を近づける
		targetAngle = atan2f(m_targetVec.x, m_targetVec.z);//移動ベクトルのx成分とz成分から、プレイヤーの向きたい方向の角度を求める
		// Y軸回転行列を作成する//この工程は毎フレーム、原点からモデルの位置に移動してから、回転する行列を作成している
		//180度ずれてたので、回転角度を180度ずらす
		/* m_rotAngle = targetAngle - DX_PI_F;*/
		 //角度の差分を計算//回転角度を-90から90にするため(最短経路を選択)
		float difference = targetAngle - m_rotAngleY - DX_PI_F;
		while (difference > DX_PI_F) difference -= 2.0f * DX_PI_F;
		while (difference < -DX_PI_F) difference += 2.0f * DX_PI_F;
		//targetAngle + DX_PI_F
		m_rotAngleY += difference * 0.1f;//回転角度を少しずつ目標の角度に近づける//ほぼlerp
	}
	//モデルは、座標の位置のcenter分下で表示

	Matrix4x4 rotY = Matrix4x4::MakeRotationY(m_rotAngleY);
	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);

	MATRIX scale = MGetScale(Vector3(2.0f,2.0f,2.0f).ToDxLibVector());
	Matrix4x4 scalemat = Matrix4x4::FromDxLibMatrix(scale);

	Matrix4x4 mtx = trans * rotY * scalemat;
	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));
	//MV1SetScale(m_modelHandle, VGet(2.0f, 2.0f, 2.0f));

	if (IsFloor())
	{
		m_accumulatedGravity = 0.0f;
	}
}
