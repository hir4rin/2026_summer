#include "EffectManager.h"
#include "DxLib.h"
#include "EffekseerForDXLib.h"

int EffectManager::Play(AsyncData type, const Vector3& pos, float rot, float scale)
{
	int resourcehandle = System::GetInstance().GetHandle(type);
	int playingHandle = PlayEffekseer3DEffect(resourcehandle);
	SetPosPlayingEffekseer3DEffect(playingHandle, pos.x,pos.y,pos.z);
	SetRotationPlayingEffekseer3DEffect(playingHandle, 0.0f, rot, 0.0f);

	if (scale != 1.0f)
	{
		SetScalePlayingEffekseer3DEffect(playingHandle, scale, scale, scale);
	}

	return playingHandle;	
}

void EffectManager::SetPos(int playingHandle, const Vector3& pos)
{
	SetPosPlayingEffekseer3DEffect(playingHandle, pos.x, pos.y, pos.z);
}

void EffectManager::SetRot(int playingHandle, float rot)
{
	SetRotationPlayingEffekseer3DEffect(playingHandle, 0.0f, rot, 0.0f);
}

void EffectManager::Stop(int playingHandle)
{
	StopEffekseer3DEffect(playingHandle);
}

bool EffectManager::IsPlaying(int playingHandle)
{
	return IsEffekseer3DEffectPlaying(playingHandle) == 1;
}
