#include "HitCol.h"

HitCol::HitCol(std::weak_ptr<CharacterBase> owner)
	: m_owner(owner)
{
	//やられ判定の初期化をOwnerがする

}

HitCol::~HitCol()
{
}

void HitCol::OnCollision(Collider& other)
{
	//何もしない
}
