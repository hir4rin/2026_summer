#include "PlayerStateSkill.h"
#include "Player.h"

PlayerStateSkill::PlayerStateSkill(std::weak_ptr<Player> player):
	PlayerState(player)
{
	//playerが既に破棄されていたら早期リターンする
	if (m_owner.expired())return;
}

PlayerStateSkill::~PlayerStateSkill()
{
}

void PlayerStateSkill::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;
	//animationの初期化//まだかいていない
	//player->m_anim.ChangeAnim(player->GetAnimName("Run"), true, 0.5f);
}

void PlayerStateSkill::Update()
{
	auto player = m_owner.lock();
	if (!player) return;
}

void PlayerStateSkill::Exit()
{
}

void PlayerStateSkill::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Skill");
}
