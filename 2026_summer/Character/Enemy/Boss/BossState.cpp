#include "BossState.h"

BossState::BossState(std::weak_ptr<BossEnemy> owner) :
	m_owner(owner)
{
}
