#pragma once
#include "PlayerState.h"
class PlayerStateJump : public PlayerState
{
public:
	PlayerStateJump(std::weak_ptr<Player> player);
	virtual ~PlayerStateJump();
	void Enter() override;
	void Update() override;
	void Exit() override;
	void DebugDraw()override;
};

