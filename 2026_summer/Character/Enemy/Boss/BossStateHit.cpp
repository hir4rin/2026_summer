#include "BossStateHit.h"
#include "BossEnemy.h"
#include "BossStateIdle.h"
#include "BossStateDead.h"
#include "../../Player/Base/Player.h"
#include "../../../Game.h"
#include "../System.h"

namespace
{
	const std::string kHitName = "Player|hit";
	const std::string kKirimomi = "Player|kirimomi";

	constexpr float kEnemyHitBackTime = 30.0f;//敵が攻撃を受けたときの、のけぞりの時間
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
	if (boss->m_attackData.isKirimomi)
	{
		boss->m_anim.ChangeAnim(kKirimomi, false, 0.8f);
	}
}

void BossStateHit::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//キリモミ攻撃で体力が0になっていたら、のけぞり中でも死亡させる
	if (boss->m_isDieOut)
	{
		boss->m_isDead = true;
		boss->ChangeState(std::make_shared<BossStateDead>(m_owner));
		return;
	}

	auto player = boss->m_player.lock();
	if (!player)return;

	//のけぞり時間のカウント//ボスは吹き飛ばず、その場でのけぞるだけ
	float timeScale = System::GetInstance().GetTimeScale();
	boss->m_knockBackFrame += 1.0f * timeScale * boss->m_ownTimeScale;

	if (boss->m_knockBackFrame <= boss->m_attackData.knockBackFrame)
	{
		//Enemy->Playerのベクトルに吹き飛ばす力を加える//プレイヤーの正面に行くようにknockBackする
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
		boss->ChangeState(std::make_shared<BossStateIdle>(m_owner));
		return;
	}
}

void BossStateHit::Exit()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	boss->m_knockBackFrame = 0;
	boss->m_vel = Vector3(0, 0, 0);
}

void BossStateHit::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Hit");
}
