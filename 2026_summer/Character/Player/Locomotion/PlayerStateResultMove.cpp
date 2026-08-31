#include "PlayerStateResultMove.h"
#include "Player.h"

PlayerStateResultMove::PlayerStateResultMove(std::weak_ptr<Player> player):
	PlayerState(player)
{
	//playerが既に破棄されていたら早期リターンする
	if (m_owner.expired())return;
}

PlayerStateResultMove::~PlayerStateResultMove()
{
}

void PlayerStateResultMove::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;
	//アニメーションを流す
	player->m_anim.ChangeAnim("Player|AirAttack3", false, 0.7f);

}

void PlayerStateResultMove::Update()
{
	auto player = m_owner.lock();
	if (!player) return;

	//アニメーションが5割を超えたらアニメーションを止める
	if (player->m_anim.GetAnimRate() > 0.6f)
	{
		//player->m_anim.
	}
	else
	{
		player->m_anim.Update();
	}
}

void PlayerStateResultMove::Exit()
{
	auto player = m_owner.lock();
	if (!player) return;
}

void PlayerStateResultMove::DebugDraw()
{
}
