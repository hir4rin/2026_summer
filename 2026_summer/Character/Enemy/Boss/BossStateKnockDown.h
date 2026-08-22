#pragma once
#include "BossState.h"

class BossStateKnockDown :
	public BossState
{
public:
	BossStateKnockDown(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateKnockDown();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
