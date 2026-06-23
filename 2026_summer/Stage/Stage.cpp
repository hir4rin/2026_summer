#include "Stage.h"

Stage::Stage()
{
	m_stageModelHandle = MV1LoadModel("data/Stage/Stage.mv1");
	m_pos = Vector3(0, 0, 0);
	ColInit(m_pos, Vector3(0, 0, 0), 0.0f, ColliderType::Polygon, Tags::StaticObject, true);
	// モデルのポリゴンの当たり判定を構築する(第二引数を-1にすると全てのポリゴンを対象にする)
	MV1SetupCollInfo(m_stageModelHandle, -1);
}

Stage::~Stage()
{
	MV1DeleteModel(m_stageModelHandle);
}

void Stage::Update()
{
	// 毎フレーム衝突情報を更新
	MV1RefreshCollInfo(m_stageModelHandle, -1);
}

void Stage::Draw() const
{
	MV1DrawModel(m_stageModelHandle);
}

void Stage::OnCollision(Collider& other)
{
	//何もしない
}