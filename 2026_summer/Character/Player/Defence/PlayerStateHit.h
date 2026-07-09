#pragma once
#include "PlayerState.h"

struct AttackData;

class PlayerStateHit :
    public PlayerState
{
public:
    PlayerStateHit(std::weak_ptr<Player> player, const AttackData& data);
    virtual ~PlayerStateHit();
    void Enter() override;
    void Update() override;
    void Exit() override;
	void DebugDraw()override;
};

