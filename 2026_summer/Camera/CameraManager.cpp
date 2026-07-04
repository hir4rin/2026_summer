#include "CameraManager.h"
#include "Camera.h"
#include "MainCamera.h"
#include "PlayerCamera.h"
#include "Movie1Camera.h"
#include "UltCamera.h"
#include "LockOnCamera.h"
#include "../System.h"
#include "EffekseerForDXLib.h"
#include "../SubWindow/SubWindow.h"

CameraManager::CameraManager()
{
	m_mainCamera = std::make_shared<MainCamera>();

	m_movieCamera = std::make_shared<Movie1Camera>();

	m_playerCamera = std::make_shared<PlayerCamera>();
	m_playerCamera->Init();

	m_ultCamera = std::make_shared<UltCamera>();
	m_LockOnCamera = std::make_shared<LockOnCamera>();

	EntryCamera(m_playerCamera);
	EntryCamera(m_movieCamera);
	EntryCamera(m_ultCamera);
	EntryCamera(m_LockOnCamera);
	//weak_ptrが必要なカメラをまとめる
	m_weakRefCameras.push_back(m_mainCamera);
	m_weakRefCameras.push_back(m_playerCamera);
	m_weakRefCameras.push_back(m_ultCamera);
	m_weakRefCameras.push_back(m_LockOnCamera);

	//初期化
	highestPriorityCamera = m_playerCamera;
	

	m_context = std::make_shared<CameraContext>();
	//MainCameraに情報を渡す
	SetUpMainCamera();
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
void CameraManager::Init(std::weak_ptr<Player> player)
{
	//プレイヤーカメラの初期化
	SetWeakRef(player);
	//playerCamera->PlayerSet(player);
	//ultCamera->
	
	for(auto& camera : m_cameras)
	{
		camera->SetCameraManager(shared_from_this());
	}
}

void CameraManager::Update(Vector3 pos, Vector3 pos2)
{

	if (m_cameras.empty())return;
	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();


	m_isPrevUltimating = m_isUltimating;
	//ウルト演出かどうか
	m_isUltimating = System::GetInstance().GetIsUltimating();
	if (m_isUltimating && !m_isPrevUltimating)
	{
		SetNextCameraPriority(Camera::Type::UltCamera,false,true);
		//m_mainCamera->SetLerp(true);
	}


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
	//もしPriority一番高いものがPlayerCameraだったら、LockOnCameraを一番上にする//そこで、m_isLockOnがtrueだったら、LockOnCameraを一番上にする
	if (highestPriorityCamera->GetCameraType() == Camera::Type::PlayerCamera)
	{
		if (m_mainCamera->GetIsLockOn())
		{
			for (auto& camera : m_cameras)
			{
				if (camera->GetCameraType() == Camera::Type::LockOnCamera)
				{
					highestPriorityCamera = camera;
					break;
				}
			}
		}
	}

	//priorityが高いカメラのUpdateを呼ぶ
	//highestPriorityCamera->Update(pos, pos2);

	for (auto& camera : m_cameras)
	{
		//すべてのカメラのUpdateを呼ぶ
		camera->Update(pos, pos2);
	}


	//MainCameraに情報を渡す
	CameraData data;
		data.pos = highestPriorityCamera->GetCameraPos();
		data.target = highestPriorityCamera->GetCameraTarget();
	//MainCameraに情報を渡す
	m_mainCamera->Update(data);
	//SetUpMainCamera();

}
void CameraManager::Draw()
{
	for (auto& camera : m_weakRefCameras)
	{
		camera->Draw();
	}
	int num = static_cast<int>(highestPriorityCamera->GetCameraType());
	std::string text = std::to_string(num);
	SubWindow::AddText("CameraType:" + text);
}
void CameraManager::ApplyCameraSettings()
{
	m_mainCamera->CameraSetting();
}

void CameraManager::SetNextCameraPriority(Camera::Type type,bool isLerp,bool isSlerp)
{
	//角度を抽出して、再び渡す
	float angleH = 0.0f;
	float angleV = 0.0f;
	////プレイヤーCameraを探して、角度を抽出する
	//for (auto& camera : m_cameras)
	//{
	//	if (camera->GetCameraType() == Camera::Type::PlayerCamera)
	//	{
	//		angleH = camera->GetCameraAngleH();
	//		angleV = camera->GetCameraAngleV();
	//		break;

	//	}
	//}
	//最もpriorityの高いカメラの値をゲットして+1して指定したものに渡す
	int nextPriority = highestPriorityCamera->GetPriority() + 1;

	//numの番号のもの以外は0にする//numの番号のものはnumにする
	for (auto& Camera : m_cameras)
	{
		if (Camera->GetCameraType() == type)//numの番号のものはnumにする
		{
			Camera->SetPriority(nextPriority);
			//MainCameraに情報を渡す

			CameraData data = Camera->GetCameraData();
			data.pos = Camera->GetCameraPos();
			data.target = Camera->GetCameraTarget();
			m_mainCamera->SetCameraData(data);
		}
		else
		{

		}
	}
	if (isLerp)
	{
		m_mainCamera->SetLerp(true);
		m_mainCamera->SetSlerp(false);
	}
	else if (isSlerp)
	{
		m_mainCamera->SetSlerp(true);
		m_mainCamera->SetLerp(false);
	}
	else
	{
		m_mainCamera->SetLerp(false);
		m_mainCamera->SetSlerp(false);
	}
}

void CameraManager::SetWeakRef(std::weak_ptr<Player> m_player, std::weak_ptr<EnemyBase> m_enemy)
{
	//カメラコンテキストにセット
	m_context->m_player = m_player;
	m_context->m_targetEnemy = m_enemy;

	for(auto& camera : m_weakRefCameras)
	{
		camera->SetCameraContext(m_context);
	}

}
void CameraManager::SetPlayerCameraAngle(float angleH, float angleV)
{
	for (auto& camera : m_cameras)
	{
		if (camera->GetCameraType() == Camera::Type::PlayerCamera)
		{
			camera->SetCameraAngle(angleH, angleV);
			break;
		}
	}
}
void CameraManager::SetUpMainCamera()
{
	//MainCameraに情報を渡す
	CameraData data;
	data.pos = highestPriorityCamera->GetCameraPos();
	data.target = highestPriorityCamera->GetCameraTarget();
	//MainCameraに情報を渡す
	m_mainCamera->Update(data);	
}


