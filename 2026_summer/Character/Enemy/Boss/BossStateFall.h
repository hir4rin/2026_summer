#pragma once
#include "BossState.h"

class BossStateFall :
	public BossState
{
public:
	BossStateFall(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateFall();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
