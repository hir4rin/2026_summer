#include "BossStateHit.h"
#include "BossEnemy.h"
#include "BossStateAirStay.h"
#include "BossStateIdle.h"
#include "BossStateDead.h"
#include "BossStateKnockDown.h"
#include "../../Player/Base/Player.h"
#include "../../../Game.h"
#include "../System.h"

namespace
{
	const std::string kHitName = "Player|hit";
	const std::string kKirimomi = "Player|kirimomi";

	constexpr float kEnemyHitBackTime = 30.0f;//敵が攻撃を受けたときの吹き飛ばしの時間
	constexpr float kEnemyDistance = 50.0f;
	constexpr float kToTargetPower = 3.0f;//プレイヤーの正面に行くようにknockBackする力
}

BossStateHit::BossStateHit(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateHit::~BossStateHit()
{
}

void BossStateHit::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	boss->m_anim.ChangeAnim(kHitName, false);
	if (boss->m_knockBackVel.y > 0.0f)
	{
		boss->m_hitType = EnemyBase::HitType::Air;
		boss->SetIsFloor(false);//空中にいるので、床にいないことにする
	}
	else if (boss->m_knockBackVel.y < 0.0f)boss->m_hitType = EnemyBase::HitType::Drop;
	else boss->m_hitType = EnemyBase::HitType::Ground;
	if (boss->m_attackData.isKirimomi)
	{
		boss->m_anim.ChangeAnim(kKirimomi, false, 0.8f);
		boss->m_hitType = EnemyBase::HitType::Drop;//吹き飛ぶときはDropにする
	}
}

void BossStateHit::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//m_knockBackVelで保存したベクトルをm_velに追加
	//y軸があるときとないときで処理を変える
	float timeScale = System::GetInstance().GetTimeScale();
	boss->m_knockBackFrame += 1.0f * timeScale * boss->m_ownTimeScale;

	switch (boss->m_hitType)
	{
		case EnemyBase::HitType::Air:
			//空中にいるときは、y軸の吹き飛ばしの力を減衰させる
			boss->m_accumulatedGravity += Game::kGravity * timeScale * boss->m_ownTimeScale;
			//速度を指定
			boss->m_vel = boss->m_knockBackVel + Vector3(0, -boss->m_accumulatedGravity, 0);
			if (boss->m_vel.y <= 0.0f)
			{
				boss->m_knockBackVel.y = 0.0f;
				boss->m_vel.y = 0.0f;
				boss->m_accumulatedGravity = 0.0f;//重力の累積値をリセット
				boss->FinishHitProcess();
				//吹き飛ばしのベクトルが0になったらAirStayに戻す
				boss->ChangeState(std::make_shared<BossStateAirStay>(m_owner));
				return;
			}
			break;
		case EnemyBase::HitType::Ground://上下差がないとき
		{//バイパス防止(switch文の中で初期化するのはコンパイルエラーになるので、波括弧でくくって明示する)
			//PlayerとEnemyのベクトル方向にとばす
			auto player = boss->m_player.lock();
			if (!player)return;
			//敵を移動させる
			if (boss->m_knockBackFrame <= boss->m_attackData.knockBackFrame)
			{
				//Enemy->Playerのベクトルに吹き飛ばす力を加える//プレイヤーの正面に行くようにknockBackする//いずれkirimomi吹っ飛びの時の処理と分ける
				Vector3 front = player->GetTargetVec();
				Vector3 pos = player->GetPos() + player->GetVel();
				Vector3 TargetPos = pos + front * kEnemyDistance;
				Vector3 toTarget = (TargetPos - boss->m_pos).Normalize() * kToTargetPower;

				Vector3 knockBackDir = (boss->m_pos - player->GetPos()).Normalize();
				knockBackDir.y = 0.0f;//y軸の吹き飛ばしはなし
				knockBackDir += toTarget;
				boss->m_vel += knockBackDir * boss->m_attackData.knockBackPower.x;
			}

			if (boss->m_knockBackFrame > kEnemyHitBackTime)//本来はplayerの攻撃終了タイミングを読み取って、そこから変わる
			{
				boss->FinishHitProcess();
				//現在地上か空中かで分岐//床についているかどうかで分岐
				if (boss->IsFloor())
				{
					boss->ChangeState(std::make_shared<BossStateIdle>(m_owner));
				}
				else
				{
					boss->ChangeState(std::make_shared<BossStateAirStay>(m_owner));
				}
				return;
			}
		}
			break;
		case EnemyBase::HitType::Drop:
			boss->m_accumulatedGravity += Game::kGravity * timeScale * boss->m_ownTimeScale;
			//速度を指定
			boss->m_vel = boss->m_knockBackVel + Vector3(0, -boss->m_accumulatedGravity, 0);

			if (boss->IsFloor())
			{
				if (boss->m_isDieOut)
				{
					boss->m_isDead = true;
					boss->ChangeState(std::make_shared<BossStateDead>(m_owner));
					return;
				}

				boss->m_vel.y = 0.0f;
				boss->m_accumulatedGravity = 0.0f;//重力の累積値をリセット
				boss->m_knockBackVel.y = 0.0f;

				boss->FinishHitProcess();
				//地面についたらChange//knockDown状態とか作ったっていい
				boss->ChangeState(std::make_shared<BossStateKnockDown>(m_owner));
				return;
			}
			break;
		default:
			break;
	}
}

void BossStateHit::Exit()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	//被弾中にもう一度Hitに入る場合はFinishHitProcessを通らないので、ここでリセットしておく
	//(通常の遷移ではFinishHitProcessが既にリセット済みなので、ここでの再代入は無害)
	boss->m_knockBackFrame = 0;
	boss->m_hitType = EnemyBase::HitType::None;
	boss->m_vel = Vector3(0, 0, 0);
}

void BossStateHit::DebugDraw()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	std::string hitTypeString;
	switch (boss->m_hitType)
	{
	case EnemyBase::HitType::None:
		hitTypeString = "None";
		break;
	case EnemyBase::HitType::Air:
		hitTypeString = "Air";
		break;
	case EnemyBase::HitType::Ground:
		hitTypeString = "Ground";
		break;
	case EnemyBase::HitType::Drop:
		hitTypeString = "Drop";
		break;
	default:
		break;
	}
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Hit");
	DrawFormatString(Game::kScreenWidth - 600, 30, GetColor(255, 255, 255), "Boss HitType: %s", hitTypeString.c_str());
}
