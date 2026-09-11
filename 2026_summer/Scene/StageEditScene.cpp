#include "StageEditScene.h"
#include "SceneController.h"
#include "../Managers/CollisionManager.h"
#include "../Stage/StageObject.h"
#include "../Stage/StageCsvIO.h"
#include "DxLib.h"
#include <cstdio>

#ifdef _DEBUG
#include "../imguiApp.h"
#include "imgui.h"
#endif

namespace
{
	constexpr float kCameraViewAngle = DX_PI_F / 3.0f;	//カメラの視野角(Camera.cppの値に合わせている)
	constexpr float kCameraNear = 10.0f;				//ニアクリップ
	constexpr float kCameraFar = 10000.0f;				//ファークリップ(編集中はカメラを大きく引くことがあるため広めにとる)

	constexpr int kGridHalfCount = 20;					//地面グリッドの、中心から片側に引く線の本数
	constexpr float kGridCellSize = 100.0f;			//地面グリッド1マスのサイズ
	constexpr unsigned int kGridColor = 0x606060u;		//地面グリッドの色

	constexpr unsigned int kPreviewColor = 0x00ffffu;	//ドラッグ中のプレビューの色(水色)
}

StageEditScene::StageEditScene(SceneController& controller) : Scene(controller)
{
	m_updateFunc = static_cast<UpdateFunc_t>(&StageEditScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&StageEditScene::NormalDraw);

	//このシーン専用のCollisionManagerとして初期化する(GameSceneなど他のシーンと同じ流儀)
	CollisionManager::GetInstance().Init();

	LoadStage();
}

StageEditScene::~StageEditScene()
{
	ClearPlacedObjects();
	CollisionManager::GetInstance().Terminate();
}

void StageEditScene::Update()
{
	(this->*m_updateFunc)();
}

void StageEditScene::FadeInUpdate()
{
}

void StageEditScene::NormalUpdate()
{
#ifdef _DEBUG
	//カメラ位置はimguiAppのカメラデバッグ用の値をそのまま使う(このシーン専用のカメラは持たない)
	imguiApp::GetInstance().DrawCameraDebugWindow(
		imguiApp::GetInstance().GetOverrideCameraPos(),
		imguiApp::GetInstance().GetOverrideCameraTarget());

	DrawEditorUI();
#endif

	if (m_isEditMode)
	{
		UpdateEditInput();
	}

	CollisionManager::GetInstance().Update();
}

void StageEditScene::FadeOutUpdate()
{
}

void StageEditScene::Draw()
{
	(this->*m_drawFunc)();
}

void StageEditScene::FadeInDraw()
{
}

void StageEditScene::NormalDraw()
{
#ifdef _DEBUG
	const Vector3 camPos = imguiApp::GetInstance().GetOverrideCameraPos();
	const Vector3 camTarget = imguiApp::GetInstance().GetOverrideCameraTarget();
	SetCameraPositionAndTarget_UpVecY(camPos.ToDxLibVector(), camTarget.ToDxLibVector());
	SetupCamera_Perspective(kCameraViewAngle);
	SetCameraNearFar(kCameraNear, kCameraFar);
#endif

	//地面の目安になるグリッドを描画する
	for (int i = -kGridHalfCount; i <= kGridHalfCount; ++i)
	{
		const float offset = i * kGridCellSize;
		const float edge = kGridHalfCount * kGridCellSize;
		DrawLine3D(
			VGet(offset, kGroundPlaneY, -edge), VGet(offset, kGroundPlaneY, edge),
			GetColor((kGridColor >> 16) & 0xff, (kGridColor >> 8) & 0xff, kGridColor & 0xff));
		DrawLine3D(
			VGet(-edge, kGroundPlaneY, offset), VGet(edge, kGroundPlaneY, offset),
			GetColor((kGridColor >> 16) & 0xff, (kGridColor >> 8) & 0xff, kGridColor & 0xff));
	}

	for (const auto& obj : m_placedObjects)
	{
		obj->Draw();
	}

	if (m_isEditMode)
	{
		DrawDraggingPreview();
	}
}

void StageEditScene::FadeOutDraw()
{
}

void StageEditScene::UpdateEditInput()
{
	const bool isPressed = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

	if (isPressed && !m_wasMousePressed)
	{
		//押し始め:ドラッグ開始
		m_isDragging = true;
		m_dragStartPos = GetMouseGroundPos(kGroundPlaneY);
	}
	else if (isPressed && m_isDragging)
	{
		//押している間:footprintの現在地を更新
		m_dragCurrentPos = GetMouseGroundPos(kGroundPlaneY);
	}
	else if (!isPressed && m_isDragging)
	{
		//離した:footprintが十分な大きさならBOXを生成して確定する
		m_isDragging = false;

		const float sizeX = fabsf(m_dragCurrentPos.x - m_dragStartPos.x);
		const float sizeZ = fabsf(m_dragCurrentPos.z - m_dragStartPos.z);

		if (sizeX >= kMinFootprintSize && sizeZ >= kMinFootprintSize)
		{
			const Vector3 center(
				(m_dragStartPos.x + m_dragCurrentPos.x) * 0.5f,
				m_createBaseY,
				(m_dragStartPos.z + m_dragCurrentPos.z) * 0.5f);
			const Vector3 halfExtents(sizeX * 0.5f, m_createHeight * 0.5f, sizeZ * 0.5f);

			auto object = std::make_shared<StageObject>();
			object->Init(center, halfExtents);
			m_placedObjects.push_back(object);

			m_selectedIndex = static_cast<int>(m_placedObjects.size()) - 1;
		}
	}

	m_wasMousePressed = isPressed;
}

void StageEditScene::DrawEditorUI()
{
#ifdef _DEBUG
	ImGui::Begin("Stage Editor");

	ImGui::Checkbox("Edit Mode", &m_isEditMode);
	ImGui::TextDisabled("Edit Mode中は左クリック長押しでBOXを配置できます");

	ImGui::Separator();
	ImGui::DragFloat("Create Height", &m_createHeight, 1.0f, 1.0f, 2000.0f);
	ImGui::DragFloat("Create Base Y", &m_createBaseY, 1.0f, -2000.0f, 2000.0f);

	ImGui::Separator();
	ImGui::InputInt("Stage Number", &m_stageNumber);
	if (m_stageNumber < 1)m_stageNumber = 1;

	if (ImGui::Button("Load"))
	{
		LoadStage();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		SaveStage();
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear All"))
	{
		ClearPlacedObjects();
	}

	ImGui::Separator();
	ImGui::Text("Placed Objects: %d", static_cast<int>(m_placedObjects.size()));

	if (ImGui::BeginListBox("##ObjectList"))
	{
		for (int i = 0; i < static_cast<int>(m_placedObjects.size()); ++i)
		{
			const bool isSelected = (m_selectedIndex == i);
			char label[32];
			sprintf_s(label, "Box %d", i);
			if (ImGui::Selectable(label, isSelected))
			{
				m_selectedIndex = i;
			}
		}
		ImGui::EndListBox();
	}

	if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_placedObjects.size()))
	{
		auto& selected = m_placedObjects[m_selectedIndex];
		Vector3 pos = selected->GetPos();
		Vector3 halfExtents = selected->GetBoxHalfExtents();

		bool changed = false;
		changed |= ImGui::DragFloat3("Position", &pos.x, 1.0f);
		changed |= ImGui::DragFloat3("Half Extents", &halfExtents.x, 1.0f, 1.0f, 5000.0f);
		if (changed)
		{
			selected->SetTransform(pos, halfExtents);
		}

		if (ImGui::Button("Delete Selected"))
		{
			CollisionManager::GetInstance().ReleaseCollider(selected);
			m_placedObjects.erase(m_placedObjects.begin() + m_selectedIndex);
			m_selectedIndex = -1;
		}
	}

	ImGui::End();
#endif
}

void StageEditScene::LoadStage()
{
	ClearPlacedObjects();

	const std::vector<StageObjectData> objects = StageCsvIO::Load(m_stageNumber);
	for (const auto& data : objects)
	{
		auto object = std::make_shared<StageObject>();
		object->Init(data.position, data.halfExtents);
		m_placedObjects.push_back(object);
	}

	m_selectedIndex = -1;
}

void StageEditScene::SaveStage()
{
	std::vector<StageObjectData> objects;
	objects.reserve(m_placedObjects.size());

	for (const auto& obj : m_placedObjects)
	{
		StageObjectData data;
		data.position = obj->GetPos();
		data.halfExtents = obj->GetBoxHalfExtents();
		objects.push_back(data);
	}

	StageCsvIO::Save(m_stageNumber, objects);
}

void StageEditScene::ClearPlacedObjects()
{
	for (auto& obj : m_placedObjects)
	{
		CollisionManager::GetInstance().ReleaseCollider(obj);
	}
	m_placedObjects.clear();
	m_selectedIndex = -1;
}

Vector3 StageEditScene::GetMouseGroundPos(float planeY) const
{
	int mouseX = 0;
	int mouseY = 0;
	GetMousePoint(&mouseX, &mouseY);

	const Vector3 nearPos = Vector3::FromDxLibVector(
		ConvScreenPosToWorldPos(VGet(static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f)));
	const Vector3 farPos = Vector3::FromDxLibVector(
		ConvScreenPosToWorldPos(VGet(static_cast<float>(mouseX), static_cast<float>(mouseY), 1.0f)));

	const Vector3 dir = farPos - nearPos;
	if (fabsf(dir.y) <= 0.0001f)
	{
		//視線が地面と平行に近い場合は、位置がほぼ求まらないためnearPosで代用する
		return nearPos;
	}

	const float t = (planeY - nearPos.y) / dir.y;
	return nearPos + dir * t;
}

void StageEditScene::DrawDraggingPreview() const
{
	if (!m_isDragging)return;

	const float sizeX = fabsf(m_dragCurrentPos.x - m_dragStartPos.x);
	const float sizeZ = fabsf(m_dragCurrentPos.z - m_dragStartPos.z);
	if (sizeX < kMinFootprintSize || sizeZ < kMinFootprintSize)return;

	const Vector3 center(
		(m_dragStartPos.x + m_dragCurrentPos.x) * 0.5f,
		m_createBaseY,
		(m_dragStartPos.z + m_dragCurrentPos.z) * 0.5f);
	const Vector3 half(sizeX * 0.5f, m_createHeight * 0.5f, sizeZ * 0.5f);

	const Vector3 corners[8] = {
		center + Vector3(-half.x, -half.y, -half.z),
		center + Vector3(half.x, -half.y, -half.z),
		center + Vector3(half.x,  half.y, -half.z),
		center + Vector3(-half.x,  half.y, -half.z),
		center + Vector3(-half.x, -half.y,  half.z),
		center + Vector3(half.x, -half.y,  half.z),
		center + Vector3(half.x,  half.y,  half.z),
		center + Vector3(-half.x,  half.y,  half.z)
	};
	const int edges[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0},
		{4, 5}, {5, 6}, {6, 7}, {7, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	};

	const unsigned int color = GetColor((kPreviewColor >> 16) & 0xff, (kPreviewColor >> 8) & 0xff, kPreviewColor & 0xff);
	for (auto& edge : edges)
	{
		DrawLine3D(corners[edge[0]].ToDxLibVector(), corners[edge[1]].ToDxLibVector(), color);
	}
}
