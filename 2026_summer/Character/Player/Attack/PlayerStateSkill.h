#pragma once
#include "PlayerState.h"
class PlayerStateSkill :
    public PlayerState
{
public:
    PlayerStateSkill(std::weak_ptr<Player> player);
    virtual ~PlayerStateSkill();
    void Enter() override;
    void Update() override;
    void Exit() override;
	void DebugDraw() override;
};

