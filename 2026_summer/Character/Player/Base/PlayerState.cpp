#include "PlayerState.h"

PlayerState::PlayerState(std::weak_ptr<Player> owner) :
	m_owner(owner)
{
}