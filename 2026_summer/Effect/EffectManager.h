#pragma once
#include "../Math/Vector3.h"
#include "../System.h"
#include <map>
#include <vector>
#include <memory>
#include <string>

class Effect;

class EffectManager
{
public:
	//シングルトンインスタンスを取得
	static EffectManager& GetInstance()
	{
		static EffectManager instance;
		return instance;
	}
	//エフェクトの再生
	int Play(AsyncData type, const Vector3& pos,float rot = 0.0f, float scale = 1.0f, float rotX = 0.0f);

	void SetPos(int playingHandle, const Vector3& pos);
	void SetRot(int playingHandle, float rot);

	//再生中のエフェクトを停止する
	void Stop(int playingHandle);
	//再生中かどうか
	bool IsPlaying(int playingHandle);

private:
	//コンストラクタとデストラクタ
	EffectManager() = default;
	~EffectManager() = default;
	//コピーコンストラクタと代入演算子を削除して、シングルトンのインスタンスが複製されないようにする
	EffectManager(const EffectManager&) = delete;
	EffectManager& operator=(const EffectManager&) = delete;

};

