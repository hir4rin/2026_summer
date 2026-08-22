#pragma once
#include "BossState.h"

class BossStateIdle :
	public BossState
{
public:
	BossStateIdle(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateIdle();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
