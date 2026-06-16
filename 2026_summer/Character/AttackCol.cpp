#include "AttackCol.h"

AttackCol::AttackCol(AttackData& data)
{
	//AttackDataを保持
	m_attackData = &data;
}

AttackCol::~AttackCol()
{
}

void AttackCol::OnCollision(Collider& other)
{
	//idで当たったかどうかを管理する//当たったidのリストにotherのidがないとき、攻撃を当てる
	//当たっていたらotherの被ダメ処理をして、Ownerに当たったことを通知してもよい
}
