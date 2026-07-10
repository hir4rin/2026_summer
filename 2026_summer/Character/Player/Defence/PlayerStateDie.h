#pragma once
#include "PlayerState.h"

struct AttackData;

class PlayerStateDie :
    public PlayerState
{
public:
    PlayerStateDie(std::weak_ptr<Player> player, AttackData& data);
    virtual ~PlayerStateDie();
    void Enter() override;
    void Update() override;
    void Exit() override;
	void DebugDraw()override;
};

