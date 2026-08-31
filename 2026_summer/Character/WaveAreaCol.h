#pragma once
#include "Collider.h"
class WaveAreaCol :
    public Collider
{
public :
    WaveAreaCol();
    virtual ~WaveAreaCol();
	void OnCollision(Collider& other) override;
	void ApplyPos() override;

	void Draw();

	/// <summary>当たり判定の初期化と合わせて、壁エフェクトも再生する</summary>
	void Init(Vector3 pos, Vector3 offset, float radius, ColliderType type, Tags tag, bool isActive, bool isTrigger = false, float lifeTime = 0.0f);
private:
	int m_effectHandle = -1;//壁エフェクトのハンドル
	int m_playingHandle = -1;//再生中の壁エフェクトのハンドル
};

