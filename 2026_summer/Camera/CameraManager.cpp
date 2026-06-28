#include "CameraManager.h"
#include "Camera.h"
#include "PlayerCamera.h"
#include "Movie1Camera.h"
#include "EffekseerForDXLib.h"

CameraManager::CameraManager()
{

	m_movieCamera = std::make_shared<Movie1Camera>();

	m_playerCamera = std::make_shared<PlayerCamera>();
	m_playerCamera->Init();

	EntryCamera(m_playerCamera);
	EntryCamera(m_movieCamera);
	
	//初期化
	highestPriorityCamera = m_playerCamera;
}

CameraManager::~CameraManager()
{
}

void CameraManager::EntryCamera(std::shared_ptr<Camera> camera)
{
	m_cameras.push_back(camera);
}

void CameraManager::RemoveCamera(std::shared_ptr<Camera> camera)
{
	m_cameras.remove(camera);
}

void CameraManager::Update(Vector3 pos, Vector3 pos2)
{
	if (m_cameras.empty())return;
	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();


	//emplace_backで、リストの最後をUpdateするようにする//ChangeCamera関数を作ってpriorityが高いものをemplace_backするようにする
	// //なので、↓のfor文は必要なくなる
	//priorityが高いカメラを探す
	highestPriorityCamera = m_cameras.front();
	for (auto& camera : m_cameras)
	{
		if (camera->GetPriority() > highestPriorityCamera->GetPriority())
		{
			highestPriorityCamera = camera;
		}
	}
	//priorityが高いカメラのUpdateを呼ぶ
	highestPriorityCamera->Update(pos, pos2);
}

void CameraManager::ApplyCameraSettings()
{
	for (auto& camera : m_cameras)
	{
		camera->CameraSetting();
	}
}

void CameraManager::SetAllCameraPriority(Camera::Type type)
{
	//角度を抽出して、再び渡す
	float angleH = 0.0f;
	float angleV = 0.0f;
	//プレイヤーCameraを探して、角度を抽出する
	for (auto& camera : m_cameras)
	{
		if (camera->GetCameraType() == Camera::Type::PlayerCamera)
		{
			angleH = camera->GetCameraAngleH();
			angleV = camera->GetCameraAngleV();
			break;

		}
	}
	// デバッグ表示（確認後に削除）
	DrawFormatString(0, 150, GetColor(255, 0, 0), "PlayerFound:true angleH:%.3f angleV:%.3f", angleH, angleV);
	
	//numの番号のもの以外は0にする//numの番号のものはnumにする
	for (auto& Camera : m_cameras)
	{
		if (Camera->GetCameraType() == type)//numの番号のものはnumにする
		{
			Camera->SetPriority(10);
			Camera->SetCameraAngle(angleH, angleV);//角度を渡す
		}
		else
		{
			Camera->SetPriority(0);
		}

		//カメラの切り替えフラグをリセットする
		Camera->SetIsChangeCamera(false);
	}
}


