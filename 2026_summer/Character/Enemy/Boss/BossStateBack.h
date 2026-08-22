#pragma once
#include "BossState.h"

class BossStateBack :
	public BossState
{
public:
	BossStateBack(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateBack();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
