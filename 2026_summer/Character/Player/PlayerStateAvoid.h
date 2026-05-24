#pragma once
#include "PlayerState.h"
class PlayerStateAvoid :
    public PlayerState
{
public:
    PlayerStateAvoid(std::weak_ptr<Player> player);
    virtual ~PlayerStateAvoid();

    void Enter() override;
    void Update() override;
    void Exit() override;

	void DebugDraw()override;
private:
	void DetermineAvoidDirection();//回避の方向を決める関数
};

