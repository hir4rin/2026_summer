#include "BossStateFall.h"
#include "BossEnemy.h"
#include "BossStateIdle.h"
#include "../../../Game.h"
#include "../System.h"

BossStateFall::BossStateFall(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateFall::~BossStateFall()
{
}

void BossStateFall::Enter()
{
}

void BossStateFall::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//落下処理
	float timeScale = System::GetInstance().GetTimeScale();
	boss->m_accumulatedGravity += Game::kGravity * timeScale * boss->m_ownTimeScale;
	boss->m_vel.y -= boss->m_accumulatedGravity;
	if (boss->IsFloor())
	{
		boss->m_vel.y = 0.0f;
		boss->m_accumulatedGravity = 0.0f;//重力の累積値をリセット
		boss->ChangeState(std::make_shared<BossStateIdle>(m_owner));
		return;
	}
}

void BossStateFall::Exit()
{
}

void BossStateFall::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Fall");
}
