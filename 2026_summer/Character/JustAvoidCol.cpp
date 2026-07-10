#include "JustAvoidCol.h"
#include "CharacterBase.h"

JustAvoidCol::JustAvoidCol(std::weak_ptr<CharacterBase> owner)
	: m_owner(owner)
{
}

JustAvoidCol::~JustAvoidCol()
{
}

void JustAvoidCol::OnCollision(Collider& other)
{
}

void JustAvoidCol::ApplyPos()
{
	auto owner = m_owner.lock();
	if (!owner)return;
	//座標の更新
	m_pos = owner->GetPos();

	////m_pos = owner->GetPos();
}
