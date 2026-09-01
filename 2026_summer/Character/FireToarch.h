#pragma once
#include "AttackCol.h"
class FireToarch :public AttackCol
{
public:
	FireToarch(std::weak_ptr<CharacterBase> owner, const AttackData& data);
	virtual ~FireToarch();

	void Update()override;

	void OnCollision(Collider& other) override;
	void ApplyPos() override;

private:
	float m_count = 0;
	int m_effectHandle = -1;//ファイアトーチのエフェクトのハンドル
	int m_playingHandle = -1;//再生中のファイアトーチエフェクトのハンドル
};

