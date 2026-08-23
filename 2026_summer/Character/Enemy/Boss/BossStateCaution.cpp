#include "BossStateCaution.h"
#include "BossEnemy.h"
#include "BossStateChase.h"
#include "../../Player/Base/Player.h"
#include "../../../Game.h"
#include "../System.h"

namespace
{
	const std::string kStrafeRight = "Player|strafe_right";

	constexpr float kEnemyMeleeAttackRange = 400.0f;//敵の近接攻撃の距離
	constexpr float kEnemyCautionMaxTime = 600.0f;//敵が警戒する時間の最大値
	constexpr float kEnemyCautionUpdateIntervalDivisor = 3.0f;//警戒中の位置更新間隔の分母
}

BossStateCaution::BossStateCaution(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateCaution::~BossStateCaution()
{
}

void BossStateCaution::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	auto player = boss->m_player.lock();
	if (!player)return;

	//右と左でアニメーションを変える
	boss->m_anim.ChangeAnim(boss->GetAnimName("CautionRightWalk"), true, 0.9f);
	boss->m_targetPos = player->GetPos();
	//Playerを見る
	boss->ToPlayerLook();
}

void BossStateCaution::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//Playerを見る
	boss->ToPlayerLook();

	//一定時間様子を見る
	//その後、Chaseに移行
	float timeScale = System::GetInstance().GetTimeScale();
	boss->m_cautionTime += 1.0f * timeScale * boss->m_ownTimeScale;
	if (boss->m_cautionTime > kEnemyCautionMaxTime)
	{
		boss->m_cautionTime = 0.0f;
		boss->ChangeState(std::make_shared<BossStateChase>(m_owner));
		return;
	}
	//定期的にプレイヤーの位置を更新する
	if (boss->TickInterval(boss->m_cautionUpdateTimer, kEnemyCautionMaxTime / kEnemyCautionUpdateIntervalDivisor))
	{
		boss->m_targetPos = boss->TargetPlayerPos();
	}
	boss->CautionMove(boss->m_targetPos, kEnemyMeleeAttackRange);
}

void BossStateCaution::Exit()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	boss->m_vel = Vector3(0, 0, 0);
}

void BossStateCaution::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Caution");
}
