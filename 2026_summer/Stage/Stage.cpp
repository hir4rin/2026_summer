#include "Stage.h"
#include "../Math/Matrix4x4.h"
#include "../System.h"

namespace
{
	constexpr float kStageColOffsetX = 1200.0f;
	constexpr float kStageColOffsetY = 20.0f;

	const Vector3 kTitleStagePos = Vector3(27, -345.3f, -99);
}


Stage::Stage()
{
	
}
void Stage::Init()
{
	// 当たり判定の初期化
	ColInit(m_pos, Vector3(0, 0, 0), 0.0f, ColliderType::Polygon, Tags::StaticObject, true);
}
void Stage::TitleInit()
{
	//モデルを別のもの
	m_stageViewHandle = MV1DuplicateModel(System::GetInstance().GetHandle(AsyncData::TitleStageModel));
	m_pos = Vector3(0, 0, 0);
	m_pos_graphic = Vector3(4800, -890, 0);

	MATRIX transmat_graphic = MGetTranslate(m_pos_graphic.ToDxLibVector());

	MV1SetMatrix(m_stageViewHandle, transmat_graphic);
	MV1SetScale(m_stageViewHandle, VGet(0.5f, 0.5f, 0.5f));
}

void Stage::GameInit()
{
	//m_stageModelHandle = MV1LoadModel("data/PreStage2/Stage2.mv1");
	//m_stageModelHandle = MV1LoadModel("data/PreStage2/Stage2Prev.mv1");
	m_stageModelHandle = MV1DuplicateModel(System::GetInstance().GetHandle(AsyncData::StageModelCollider));
	m_stageViewHandle = MV1DuplicateModel(System::GetInstance().GetHandle(AsyncData::StageModel));
	//m_pos = Vector3(1200, -50, 2500);
	m_pos = Vector3(kStageColOffsetX, kStageColOffsetY, 0);
	m_pos_graphic = Vector3(kStageColOffsetX, 0, 0);

	// モデルのポリゴンの当たり判定を構築する(第二引数を-1にすると全てのポリゴンを対象にする)
	MV1SetupCollInfo(m_stageModelHandle, -1);

	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);

	MATRIX transmat_graphic = MGetTranslate(m_pos_graphic.ToDxLibVector());

	MV1SetMatrix(m_stageModelHandle, Matrix4x4::ToDxLibMatrix(trans));
	MV1SetMatrix(m_stageViewHandle, transmat_graphic);
}

Stage::~Stage()
{
	MV1DeleteModel(m_stageModelHandle);
	MV1DeleteModel(m_stageViewHandle);
}

void Stage::Update()
{
	// 毎フレーム衝突情報を更新
	MV1RefreshCollInfo(m_stageModelHandle, -1);
}

void Stage::Draw() const
{
	//MV1DrawModel(m_stageModelHandle);
	MV1DrawModel(m_stageViewHandle);
}

void Stage::OnCollision(Collider& other)
{
	//何もしない
}