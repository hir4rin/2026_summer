#include "ResultCameraState.h"
#include "../CameraManager.h"

namespace
{
	constexpr float kCameraHeight = 130.0f;//カメラの高さ

	//リザルト画面でPlayerを映す定点カメラの位置・注視点(GameClearSceneでのPlayerの立ち位置(0,0,300)を基準に設定)
	const Vector3 kCameraPos = Vector3(130.0f, kCameraHeight, -80.0f);
	const Vector3 kCameraTarget = Vector3(0.0f, kCameraHeight, -80.0f);
}

ResultCameraState::ResultCameraState(std::weak_ptr<CameraManager> owner) : CameraStateBase(owner)
{
}

ResultCameraState::~ResultCameraState()
{
}

void ResultCameraState::Enter(CameraData data)
{
	m_pos = kCameraPos;
	m_target = kCameraTarget;

	//カメラの設定
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraNearFar(0.0f, 1500.0f);
}

void ResultCameraState::Update()
{
	//定点でPlayerを映すだけなので、座標・注視点は固定で更新の必要はない
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void ResultCameraState::Exit()
{
}

void ResultCameraState::CameraSetting()
{
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}
