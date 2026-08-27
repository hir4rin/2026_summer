#include "BossStateDead.h"
#include "BossEnemy.h"
#include "../../../Game.h"
#include "../../../System.h"
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
	boss->m_anim.ChangeAnim(boss->GetAnimName("Death"), false, 0.9f);
	
}

void BossStateDead::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;


	//イベント中ではないならイベント発火
	if (!System::GetInstance().GetIsLastHitEventPlaying())
	{
		if (boss->m_anim.GetAnimRate() > 0.4f)
		{
			//イベントを発火させる
			System::GetInstance().SetIsLastHitEventPlaying(true);

		}
	}
	

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
