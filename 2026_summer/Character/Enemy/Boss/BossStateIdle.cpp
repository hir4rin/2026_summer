#include "BossStateIdle.h"
#include "BossEnemy.h"
#include "BossStateAttack.h"
#include "BossStateChase.h"
#include "BossStateCaution.h"
#include "../../../Game.h"
#include "../System.h"
#include <cstdlib>

namespace
{
	const std::string kIdle = "Player|Player|Idle";

	constexpr float kEnemyIdleMaxTime = 200.0f;//敵がIdle状態でいる時間の最大値
	constexpr float kEnemyMeleeAttackRange = 400.0f;//敵の近接攻撃の距離

	constexpr int kStateChangeRandomMax = 100;//Chase/Caution遷移の抽選範囲
	constexpr int kStateChangeThreshold = 50;//Chase/Caution遷移のしきい値
}

BossStateIdle::BossStateIdle(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateIdle::~BossStateIdle()
{
}

void BossStateIdle::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	boss->m_anim.ChangeAnim(boss->GetAnimName("Idle"), false, 0.9f);
	boss->m_idleTime = 0.0f;
}

void BossStateIdle::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//Playerを見る
	boss->ToPlayerLook();

	float timeScale = System::GetInstance().GetTimeScale();
	boss->m_idleTime += 1.0f * timeScale * boss->m_ownTimeScale;
	//一定時間Idle状態でいる
	if (boss->m_idleTime < kEnemyIdleMaxTime)return;

	//ランダムでChaseかCautionに遷移する
	if (boss->CanMeleeAttack(kEnemyMeleeAttackRange))
	{
		boss->ChangeState(std::make_shared<BossStateAttack>(m_owner));
		return;
	}
	//ランダム
	if (rand() % kStateChangeRandomMax < kStateChangeThreshold)
	{
		boss->ChangeState(std::make_shared<BossStateChase>(m_owner));
		return;
	}
	else if (rand() % kStateChangeRandomMax >= kStateChangeThreshold)
	{
		boss->ChangeState(std::make_shared<BossStateCaution>(m_owner));
		//return;
	}
}

void BossStateIdle::Exit()
{
}

void BossStateIdle::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Idle");
}
