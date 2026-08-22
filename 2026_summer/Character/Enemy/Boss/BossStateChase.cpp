#include "BossStateChase.h"
#include "BossEnemy.h"
#include "BossStateAttack.h"
#include "../../Player/Base/Player.h"
#include "../../../Game.h"
#include "../System.h"

namespace
{
	const std::string kRunName = "Player|Run";

	constexpr float kEnemyMeleeAttackRange = 400.0f;//敵の近接攻撃の距離
	constexpr float kEnemyTargetUpdateTime = 30.0f;//敵がターゲットを更新する時間
}

BossStateChase::BossStateChase(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateChase::~BossStateChase()
{
}

void BossStateChase::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	auto player = boss->m_player.lock();
	if (!player)return;

	boss->m_anim.ChangeAnim(kRunName, true, 0.8f);
	boss->m_targetPos = player->GetPos();
	//Playerを見る
	boss->ToPlayerLook();
}

void BossStateChase::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//攻撃可能な距離に入ったら攻撃
	if (boss->CanMeleeAttack(kEnemyMeleeAttackRange))
	{
		boss->ChangeState(std::make_shared<BossStateAttack>(m_owner));
		return;
	}
	//定期的にプレイヤーの位置を更新する
	if (!boss->ChasePlayer(boss->m_targetPos, kEnemyMeleeAttackRange))
	{
		float timeScale = System::GetInstance().GetTimeScale();
		boss->m_chasingTime += 1.0f * timeScale * boss->m_ownTimeScale;
		//更新
		if (boss->m_chasingTime > kEnemyTargetUpdateTime)
		{
			boss->m_chasingTime = 0.0f;
			boss->m_targetPos = boss->TargetPlayerPos();
			//Playerを見る
			boss->ToPlayerLook();
		}
	}
	else
	{
		//いろいろやってもなぜか攻撃せず止まるので、距離関係なく攻撃させる
		boss->ChangeState(std::make_shared<BossStateAttack>(m_owner));
		return;
	}
}

void BossStateChase::Exit()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	boss->m_vel = Vector3(0, 0, 0);
}

void BossStateChase::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Chase");
}
