#pragma once
#include "PlayerState.h"
class PlayerStateDie :
    public PlayerState
{
public:
    PlayerStateDie(std::weak_ptr<Player> player);
    virtual ~PlayerStateDie();
    void Enter() override;
    void Update() override;
    void Exit() override;
	void DebugDraw()override;
};

