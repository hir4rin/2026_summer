#include "PlayerStateDie.h"

PlayerStateDie::PlayerStateDie(std::weak_ptr<Player> player):
	PlayerState(player)
{
	//playerが既に破棄されていたら早期リターンする//trueで破棄されている
	if (m_owner.expired())return;
}

PlayerStateDie::~PlayerStateDie()
{
}

void PlayerStateDie::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;
	//アニメーションを流す
	player->m_anim.ChangeAnim(player->GetAnimName("UltAttack"), false, 1.0f);
}

void PlayerStateDie::Update()
{
}

void PlayerStateDie::Exit()
{
}

void PlayerStateDie::DebugDraw()
{
}
