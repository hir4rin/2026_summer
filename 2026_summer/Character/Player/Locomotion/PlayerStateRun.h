#pragma once
#include "PlayerState.h"

class Input;

class PlayerStateRun : public PlayerState
{
public:
	PlayerStateRun(std::weak_ptr<Player> player);
	virtual ~PlayerStateRun();
	void Enter() override;
	void Update()override;
	void Exit() override;
	void DebugDraw() override;
private:
	void Move(Input& input);//移動処理
};

