#include "CameraManager.h"
#include "Camera.h"
#include "MainCamera.h"
#include "PlayerCamera.h"
#include "Movie1Camera.h"
#include "UltCamera.h"
#include "LockOnManager.h"
#include "TitleCamera.h"
#include "LockOnCamera.h"
#include "../Managers/CollisionManager.h"
#include "../System.h"
#include "../Character/Enemy/EnemyBase.h"
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
	m_TitleCamera = std::make_shared<TitleCamera>();

	EntryCamera(m_playerCamera);
	EntryCamera(m_movieCamera);
	EntryCamera(m_ultCamera);
	EntryCamera(m_LockOnCamera);
	EntryCamera(m_TitleCamera);
	//weak_ptrが必要なカメラをまとめる
	m_weakRefCameras.push_back(m_mainCamera);
	m_weakRefCameras.push_back(m_playerCamera);
	m_weakRefCameras.push_back(m_ultCamera);
	m_weakRefCameras.push_back(m_LockOnCamera);

	//初期化
	highestPriorityCamera = m_playerCamera;
	
	m_context = std::make_shared<CameraContext>();

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
void CameraManager::Init(std::weak_ptr<Player> player,std::weak_ptr<Stage> stage)
{
	//プレイヤーカメラの初期化
	SetWeakRef(player);
	//playerCamera->PlayerSet(player);
	//ultCamera->
	//m_mainCameraはm_camerasに含まれないため、個別にCameraManagerの参照を渡す
	m_mainCamera->SetCameraManager(shared_from_this());
	//MainCameraに情報を渡す
	SetUpMainCamera();

	for(auto& camera : m_cameras)
	{
		camera->SetCameraManager(shared_from_this());
		camera->SetStage(stage);
	}
}

void CameraManager::Update(Vector3 pos, Vector3 pos2)
{

	if (m_cameras.empty())return;
	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();
		
	if (m_isTitle)
	{
		//タイトル画面では、他のカメラの優先度争い(LockOnCameraのPlayerCameraへの巻き戻しなど)を行わず、TitleCameraだけを使う
		highestPriorityCamera = m_TitleCamera;
		m_TitleCamera->Update(pos, pos2);

		CameraData data;
		data.pos = highestPriorityCamera->GetCameraPos();
		data.target = highestPriorityCamera->GetCameraTarget();
		m_mainCamera->Update(data);
		return;
	}

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
			SetNextCameraPriority(camera->GetCameraType(),
				camera->GetCameraSetUp().DoLerp, camera->GetCameraSetUp().DoSlerp);
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
					//Priorityを1上にする
					int nextpriority = highestPriorityCamera->GetPriority() + 1;
					camera->SetPriority(nextpriority);

					highestPriorityCamera = camera;
					//データを渡す
					CameraData data = camera->GetCameraData();
					data.pos = camera->GetCameraPos();
					data.target = camera->GetCameraTarget();
					m_mainCamera->SetCameraData(data);
					//SetNextCameraPriority(Camera::Type::LockOnCamera, false, true);
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
	//カメラ変更を反映させる
	Effekseer_Sync3DSetting();
}

void CameraManager::SetNextCameraPriority(Camera::Type type,bool isLerp,bool isSlerp)
{
	//同じだったらreturn
	if (highestPriorityCamera->GetCameraType() == type)return;



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

	//typeが一致したものを一番高いものにする
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
	//m_context->m_targetEnemy = m_enemy;

	for(auto& camera : m_weakRefCameras)
	{
		camera->SetCameraContext(m_context);
	}
	//mainCameraにもセット
	m_mainCamera->SetCameraContext(m_context);


}
void CameraManager::SetWeakTargetEnemy(int id)
{
	////idで指定したEnemyをターゲットにする
	//auto enemy = CollisionManager::GetInstance().GetColliderById(id);
	//if (!enemy)
	//{
	//	assert(false && "指定したidのEnemyが存在しません");
	//	return;
	//}
	////EnemyBaseのshared_ptrを取得
	//auto enemyBase = std::dynamic_pointer_cast<EnemyBase>(enemy);
	//if (!enemyBase)
	//{
	//	assert(false && "指定したidのEnemyはEnemyBaseではありません");
	//	return;
	//}

	//m_context->m_targetEnemy = enemyBase;
	//for (auto& camera : m_weakRefCameras)
	//{
	//	camera->SetCameraContext(m_context);
	//}
	////mainCameraにもセット
	//m_mainCamera->SetCameraContext(m_context);


}

std::shared_ptr<EnemyBase> CameraManager::GetTargetEnemy()const
{
	auto lockOnManager = m_lockOnManager.lock();
	if (!lockOnManager)return nullptr;
	return lockOnManager->GetTarget().lock();
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


