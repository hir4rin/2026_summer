#pragma once
#include "Collider.h"
#include <memory>
class CharacterBase;

class HitCol :
    public Collider
{
public:
	HitCol(std::weak_ptr<CharacterBase> owner);
	virtual ~HitCol();

	void OnCollision(Collider& other) override;

protected:
	std::weak_ptr<CharacterBase> m_owner;//当たり判定を持つキャラクターへの弱参照
};

