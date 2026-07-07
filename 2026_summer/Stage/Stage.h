#pragma once
#include "Collider.h"
class Stage : public Collider
{
public:
	Stage();
	~Stage();
	void Init();
	void Update();
	void Draw()const;
	void OnCollision(Collider& other)override;
	int GetStageModelHandle()const { return m_stageModelHandle;}

	void SetHitDim(MV1_COLL_RESULT_POLY_DIM& dim) { m_hitDim = dim; }
	MV1_COLL_RESULT_POLY_DIM& GetHitDim() { return m_hitDim; }
private:
	void ApplyPos() override {};//座標の更新はしない

	int m_stageModelHandle;
	//ポリゴンを持たせる?のかな
	//当たったポリゴンの情報
	MV1_COLL_RESULT_POLY_DIM m_hitDim;
};

