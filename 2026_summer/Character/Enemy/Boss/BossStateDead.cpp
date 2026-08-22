#include "BossStateDead.h"
#include "BossEnemy.h"
#include "../../../Game.h"

namespace
{
	const std::string kDie = "Player|Die";
}

BossStateDead::BossStateDead(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateDead::~BossStateDead()
{
}

void BossStateDead::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//死亡アニメーションを流す
	boss->m_anim.ChangeAnim(kDie, false, 0.5f);
}

void BossStateDead::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	if (boss->m_anim.GetAnimEndFlag())
	{
		boss->m_isDead = true;
	}
}

void BossStateDead::Exit()
{
}

void BossStateDead::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Dead");
}
