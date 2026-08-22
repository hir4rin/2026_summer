#include "BossStateAttack.h"
#include "BossEnemy.h"
#include "BossStateBack.h"
#include "../../../Game.h"

namespace
{
	const std::string kAttack = "Player|Attack";

	constexpr float kEnemyAttackCoolTime = 120.0f;//敵の攻撃のクールタイム
}

BossStateAttack::BossStateAttack(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateAttack::~BossStateAttack()
{
}

void BossStateAttack::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	boss->m_anim.ChangeAnim(kAttack, false, 0.9f);
	//Playerを見る
	boss->ToPlayerLook();
}

void BossStateAttack::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	boss->Attack();
	if (boss->m_anim.GetAnimEndFlag())
	{
		boss->m_attackCoolTime = kEnemyAttackCoolTime;//攻撃のクールタイムをリセット
		boss->ChangeState(std::make_shared<BossStateBack>(m_owner));
		return;
	}
}

void BossStateAttack::Exit()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	boss->m_vel = Vector3(0, 0, 0);
	boss->m_attackCol->SetIsActive(false);
	boss->m_attackCol->ClearHitIds();
}

void BossStateAttack::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Attack");
}
