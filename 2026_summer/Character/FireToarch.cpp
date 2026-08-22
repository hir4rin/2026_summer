#include "FireToarch.h"
#include "../../../System.h"


namespace
{
	constexpr float kDeppertureTime = 70.0f;
}

FireToarch::FireToarch(std::weak_ptr<CharacterBase> owner, const AttackData& data):AttackCol(owner,data)
{
	//m_lifeTimeで寿命を設定
	//数フレーム後に当たり判定をtrueにする
	m_lifeTime = 100;
	SetIsActive(false);

	//エフェクトの生成
}

FireToarch::~FireToarch()
{
}

void FireToarch::Update()
{
	float timescale = System::GetInstance().GetTimeScale();
	m_count += 1.0f * timescale;

	if (m_count > kDeppertureTime)
	{
		SetIsActive(true);
	}
}

void FireToarch::OnCollision(Collider& other)
{
	AttackCol::OnCollision(other);
}

void FireToarch::ApplyPos()
{
	
}
