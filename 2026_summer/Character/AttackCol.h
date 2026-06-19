#pragma once
#include "Collider.h"
#include <memory>
class CharacterBase;
struct AttackData;//前方宣言は値渡しができないので、参照で渡す

class AttackCol :
	public Collider
{
public:
	AttackCol(std::weak_ptr<CharacterBase> owner,const AttackData& data);
	virtual ~AttackCol();
	void OnCollision(Collider& other) override;
	void ApplyPos() override;
private:
	std::weak_ptr<CharacterBase> m_owner;//当たり判定を持つキャラクターへの弱参照
	//当たったidのリスト
	std::list<int> m_hitIds;//攻撃が当たったIDのリスト(重複ヒット防止)
	//AttackDataを保持
	std::shared_ptr<AttackData> m_attackData;
};
