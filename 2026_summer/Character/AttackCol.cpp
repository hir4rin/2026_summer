#include "AttackCol.h"
#include "CharacterBase.h"

AttackCol::AttackCol(std::weak_ptr<CharacterBase> owner,const AttackData& data)
	: m_owner(owner)
{
	auto handler = m_owner.lock();
	if (!handler)return;
	//AttackDataを保持
	m_attackData = std::make_shared<AttackData>(data);
}

AttackCol::~AttackCol()
{
}

void AttackCol::OnCollision(Collider& other)
{
	//idで当たったかどうかを管理する//当たったidのリストにotherのidがないとき、攻撃を当てる
	//当たっていたらotherの被ダメ処理をして、Ownerに当たったことを通知してもよい
	int otherId = 0;

	//if (m_hitIds.count(otherId))
	//{

	//}
}

void AttackCol::ApplyPos()
{
	auto owner = m_owner.lock();
	if (!owner)return;
	//座標の更新//所有者のvelをもらって更新
	m_pos = owner->GetPos();
	m_pos += owner->GetForward() * 150.0f;//攻撃判定を前に出す
}