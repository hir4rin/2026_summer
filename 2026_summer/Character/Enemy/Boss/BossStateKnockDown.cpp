#include "BossStateKnockDown.h"
#include "BossEnemy.h"
#include "BossStateIdle.h"
#include "../../../Game.h"
#include "../System.h"

namespace
{
	constexpr float kEnemyKnockDownFrame = 55.0f;//ダウン状態が続くフレーム数
}

BossStateKnockDown::BossStateKnockDown(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
}

BossStateKnockDown::~BossStateKnockDown()
{
}

void BossStateKnockDown::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	//knockDownフレームのリセット
	boss->m_knockBackDownFrame = 0.0f;
}

void BossStateKnockDown::Update()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	//吹き飛んだ後のダウン時間
	float timeScale = System::GetInstance().GetTimeScale();
	boss->m_knockBackDownFrame += 1.0f * timeScale * boss->m_ownTimeScale;
	if (boss->m_knockBackDownFrame > kEnemyKnockDownFrame)
	{
		boss->m_knockBackDownFrame = 0.0f;
		boss->ChangeState(std::make_shared<BossStateIdle>(m_owner));
		return;
	}
}

void BossStateKnockDown::Exit()
{
}

void BossStateKnockDown::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: KnockDown");
}
