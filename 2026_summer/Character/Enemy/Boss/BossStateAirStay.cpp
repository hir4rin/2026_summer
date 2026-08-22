#include "BossStateAirStay.h"
#include "BossEnemy.h"
#include "BossStateFall.h"
#include "../../../Game.h"

namespace
{
	constexpr float kEnemyAirTime = 90.0f;//敵が空中にとどまるる時間
}

BossStateAirStay::BossStateAirStay(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateAirStay::~BossStateAirStay()
{
}

void BossStateAirStay::Enter()
{
}

void BossStateAirStay::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	boss->m_airCount += 1.0f;
	if (boss->m_airCount > kEnemyAirTime)
	{
		boss->m_airCount = 0.0f;
		boss->FinishHitProcess();
		boss->ChangeState(std::make_shared<BossStateFall>(m_owner));
		return;
	}
}

void BossStateAirStay::Exit()
{
}

void BossStateAirStay::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: AirStay");
}
