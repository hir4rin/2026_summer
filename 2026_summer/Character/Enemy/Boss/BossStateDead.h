#pragma once
#include "BossState.h"

class BossStateDead :
	public BossState
{
public:
	BossStateDead(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateDead();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
