#pragma once
#include "PlayerState.h"
class PlayerStateParry :
    public PlayerState
{
public:
    PlayerStateParry(std::weak_ptr<Player> player);
    ~PlayerStateParry();

    void Enter() override;
    void Update() override;
    void Exit() override;

    void DebugDraw()override;
private:

};