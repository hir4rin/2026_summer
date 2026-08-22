#pragma once
#include "BossState.h"

class BossStateAttack :
	public BossState
{
public:
	BossStateAttack(std::weak_ptr<BossEnemy> boss);
	virtual ~BossStateAttack();
	void Enter() override;
	void Update() override;
	void Exit() override;

	void DebugDraw()override;

private:
	void DetermineAttackType();

	void MeleeAttack();
private:
	bool isSpawned = false;
};
