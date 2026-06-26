#include "Stage.h"
#include "../Math/Matrix4x4.h"

Stage::Stage()
{
	m_stageModelHandle = MV1LoadModel("data/PreStage/Stage.mv1");
	m_pos = Vector3(0, -700, 0);
	ColInit(m_pos, Vector3(0, 0, 0), 0.0f, ColliderType::Polygon, Tags::StaticObject, true);
	// モデルのポリゴンの当たり判定を構築する(第二引数を-1にすると全てのポリゴンを対象にする)
	MV1SetupCollInfo(m_stageModelHandle, -1);

	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);

	MV1SetMatrix(m_stageModelHandle, Matrix4x4::ToDxLibMatrix(trans));
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