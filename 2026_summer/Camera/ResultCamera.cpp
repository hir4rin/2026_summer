#include "ResultCamera.h"
#include "../SubWindow/SubWindow.h"

namespace
{
	constexpr float kCameraHeight = 130.0f;//カメラの高さ

	//リザルト画面でPlayerを映す定点カメラの位置・注視点(GameClearSceneでのPlayerの立ち位置(0,0,300)を基準に設定)
	const Vector3 kCameraPos = Vector3(130.0f, kCameraHeight, -80.0f);
	const Vector3 kCameraTarget = Vector3(0.0f, 130.0f, -80.0f);

	constexpr float kCameraViewAngle = DX_PI_F / 3.0f;//カメラの視野角
	constexpr float kCameraNear = 0.0f;//ニアクリップ
	constexpr float kCameraFar = 1500.0f;//ファークリップ
}

ResultCamera::ResultCamera()
{
	m_type = Type::ResultCamera;
	m_priority = 10;//リザルト画面ではこのカメラしか使わない
	m_pos = kCameraPos;
	m_target = kCameraTarget;
}

ResultCamera::~ResultCamera()
{
}

void ResultCamera::Init()
{
	//カメラの設定
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	SetupCamera_Perspective(kCameraViewAngle);
	SetCameraNearFar(kCameraNear, kCameraFar);
}

void ResultCamera::Draw()
{
	const std::string posText = "ResultCameraPos:(" + std::to_string(m_pos.x) + "," + std::to_string(m_pos.y) + "," + std::to_string(m_pos.z) + ")";
	SubWindow::AddText(posText);
}

void ResultCamera::Update(Vector3 pos, Vector3 pos2)
{
	//定点でPlayerを映すだけなので、座標・注視点は固定で更新の必要はない
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void ResultCamera::CameraSetting()
{
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}
