#pragma once
#include "Collider.h"

class CharacterBase;
struct AttackData;//前方宣言は値渡しができないので、参照で渡す

class AttackCol :
    public Collider
{
public:
    AttackCol(AttackData& data);
	virtual ~AttackCol();
	void OnCollision(Collider& other) override;
private:
	std::weak_ptr<CharacterBase> m_owner;//当たり判定を持つキャラクターへの弱参照
	//当たったidのリスト
	//AttackDataを保持
	AttackData* m_attackData = nullptr;
};

