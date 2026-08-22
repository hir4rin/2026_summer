#pragma once
#include "BossState.h"

class BossStateChase :
	public BossState
{
public:
	BossStateChase(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateChase();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;
};
