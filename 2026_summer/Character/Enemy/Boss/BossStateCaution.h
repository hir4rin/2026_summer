#pragma once
#include "BossState.h"

class BossStateCaution :
	public BossState
{
public:
	BossStateCaution(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateCaution();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
