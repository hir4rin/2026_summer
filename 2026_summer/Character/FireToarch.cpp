#include "FireToarch.h"
#include "../../../System.h"
#include "EffekseerForDXLib.h"


namespace
{
	constexpr float kDeppertureTime = 70.0f;
	constexpr float kLifeTime = 100.0f;//寿命
}

FireToarch::FireToarch(std::weak_ptr<CharacterBase> owner, const AttackData& data):AttackCol(owner,data)
{
	//m_lifeTimeで寿命を設定
	//数フレーム後に当たり判定をtrueにする
	m_lifeTime = kLifeTime;
	SetIsActive(false);

	//エフェクトのハンドルを取得(再生はApplyPosで座標が確定してから行う)
	m_effectHandle = System::GetInstance().GetHandle(AsyncData::BossAttackFireEffect);
}

FireToarch::~FireToarch()
{
	if (m_playingHandle != -1)
	{
		StopEffekseer3DEffect(m_playingHandle);
	}
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
	//エフェクトを再生し、位置を更新する
	if (m_playingHandle == -1)
	{
		m_playingHandle = PlayEffekseer3DEffect(m_effectHandle);
	}
	Vector3 center = GetWorldCenter();
	SetPosPlayingEffekseer3DEffect(m_playingHandle, center.x, center.y, center.z);
}
