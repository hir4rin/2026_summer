#include "BossStateBack.h"
#include "BossEnemy.h"
#include "BossStateIdle.h"
#include "../../Player/Base/Player.h"
#include "../../../Game.h"

namespace
{
	const std::string kBack = "Player|dodge_bac";

	constexpr float kEnemyBackDistance = 600.0f;//敵が距離を取るときの距離
}

BossStateBack::BossStateBack(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateBack::~BossStateBack()
{
}

void BossStateBack::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	auto player = boss->m_player.lock();
	if (!player)return;

	boss->m_anim.ChangeAnim(kBack, false, 0.5f);
	boss->m_targetPos = player->GetPos();
	//Playerを見る
	boss->ToPlayerLook();
}

void BossStateBack::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//一定距離離れたらIdle
	if (boss->BackMove(boss->m_targetPos, kEnemyBackDistance))
	{
		boss->m_idleTime = 0.0f;
		boss->ChangeState(std::make_shared<BossStateIdle>(m_owner));
		return;
	}
}

void BossStateBack::Exit()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	boss->m_vel = Vector3(0, 0, 0);
}

void BossStateBack::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Back");
}
