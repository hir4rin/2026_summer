#include "AttackCol.h"
#include "CharacterBase.h"

namespace
{
	constexpr float kAttackColOffset = 50.0f;//攻撃判定を前に出す距離//本来はここも攻撃ごとに変えるべき
}

AttackCol::AttackCol(std::weak_ptr<CharacterBase> owner,const AttackData& data)
	: m_owner(owner)
{
	if (m_owner.expired())return;
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

	//Tag処理
	if(other.GetTag() == Tags::PlayerHit || other.GetTag() == Tags::EnemyHit)
	{
		int otherId = other.GetId();
		auto it = std::find(m_hitIds.begin(), m_hitIds.end(), otherId);
		if (it == m_hitIds.end())
		{
			// 当たっていない場合の処理
			//otherの被ダメ処理
			m_hitIds.push_back(otherId);//当たったidのリストにotherのidを追加する
			
		}

	}
	else
	{
		// 当たっている場合の処理
		return;
	}
}

void AttackCol::ApplyPos()
{
	auto owner = m_owner.lock();
	if (!owner)return;
	//座標の更新//所有者のvelをもらって更新
	m_pos = owner->GetPos();
	//m_attackDataに基づいて、このkAttackColOffsetを変えるようにしないといけない
	m_pos += owner->GetTargetVec() * kAttackColOffset;//攻撃判定を前に出す
}