#pragma once
#include "BossState.h"

class BossStateHit :
	public BossState
{
public:
	BossStateHit(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateHit();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
