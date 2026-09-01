#include "WaveAreaCol.h"
#include "../System.h"
#include "EffekseerForDXLib.h"

WaveAreaCol::WaveAreaCol()
{
}

WaveAreaCol::~WaveAreaCol()
{
	if (m_playingHandle != -1)
	{
		StopEffekseer3DEffect(m_playingHandle);
	}
}

void WaveAreaCol::Init(Vector3 pos, Vector3 offset, float radius, ColliderType type, Tags tag, bool isActive, bool isTrigger, float lifeTime)
{
	ColInit(pos, offset, radius, type, tag, isActive, isTrigger, lifeTime);

	//壁エフェクトを再生する
	m_effectHandle = System::GetInstance().GetHandle(AsyncData::AreaWallEffect);
	m_playingHandle = PlayEffekseer3DEffect(m_effectHandle);
	Vector3 center = GetWorldCenter();
	SetPosPlayingEffekseer3DEffect(m_playingHandle, center.x, center.y, center.z);
}

void WaveAreaCol::OnCollision(Collider& other)
{
}

void WaveAreaCol::ApplyPos()
{
	//壁エフェクトの位置を更新する
	if (m_playingHandle == -1)return;
	Vector3 center = GetWorldCenter();
	SetPosPlayingEffekseer3DEffect(m_playingHandle, center.x, center.y, center.z);
	if (IsEffekseer3DEffectPlaying(m_playingHandle))
	{
		m_playingHandle = PlayEffekseer3DEffect(m_effectHandle);
		Vector3 center = GetWorldCenter();
		SetPosPlayingEffekseer3DEffect(m_playingHandle, center.x, center.y, center.z);
	}
}
void WaveAreaCol::Draw()
{
#ifdef _DEBUG
	//デバッグ描画
	DebugDraw();
#endif
}
