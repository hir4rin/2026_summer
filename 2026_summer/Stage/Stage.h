#pragma once
#include "Collider.h"
class Stage : public Collider
{
public:
	Stage();
	~Stage();
	void Init();
	void TitleInit();
	void GameInit();

	void Update();
	void Draw()const;
	void OnCollision(Collider& other)override;
	int GetStageModelHandle()const { return m_stageModelHandle;}

	void SetHitDim(MV1_COLL_RESULT_POLY_DIM& dim) { m_hitDim = dim; }
	MV1_COLL_RESULT_POLY_DIM& GetHitDim() { return m_hitDim; }
private:
	void ApplyPos() override {};//座標の更新はしない

	Vector3 m_pos_graphic;

	int m_stageModelHandle;
	int m_stageViewHandle;
	//当たったポリゴンの情報
	MV1_COLL_RESULT_POLY_DIM m_hitDim;
};

