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

	int m_hadouEffectHandle = -1;//はどう攻撃のエフェクトのハンドル
	int m_hadouPlayingHandle = -1;//再生中のはどう攻撃エフェクトのハンドル

	int m_summonEffectHandle = -1;//召喚攻撃のエフェクトのハンドル
};
