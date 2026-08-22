#pragma once
#include "BossState.h"

class BossStateAirStay :
	public BossState
{
public:
	BossStateAirStay(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateAirStay();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
