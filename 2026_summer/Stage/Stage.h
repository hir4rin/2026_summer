#pragma once
#include "Collider.h"
class Stage : public Collider
{
public:
	Stage();
	~Stage();
	void Update();
	void Draw()const;
	void OnCollision(Collider& other)override;
	int GetStageModelHandle()const { return m_stageModelHandle; }
private:
	void ApplyPos() override {};//座標の更新はしない

	int m_stageModelHandle;
	//ポリゴンを持たせる?のかな
};

