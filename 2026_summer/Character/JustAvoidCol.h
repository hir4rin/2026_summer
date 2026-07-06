#pragma once
#include "Collider.h"

class JustAvoidCol : public Collider
{
public:
	JustAvoidCol(std::weak_ptr<CharacterBase> owner);
	virtual ~JustAvoidCol();

	void OnCollision(Collider& other) override;
	void ApplyPos() override;
protected:
	std::weak_ptr<CharacterBase> m_owner;//当たり判定を持つキャラクターへの弱参照

};

