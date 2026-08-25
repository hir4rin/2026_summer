#include "LockOnCameraState.h"
#include "../CameraManager.h"
#include "../LockOnManager.h"
#include "PlayerFollowCamera.h"
#include "../Character/Enemy/EnemyBase.h"
#include "Player.h"
#include "../MainCamera.h"
#include "../Stage/Stage.h"
#include "../Math/Matrix4x4.h"
#include <algorithm>

namespace
{
	constexpr float kToPlayerLength = 350.0f;//プレイヤーからカメラまでの距離
	constexpr float kCameraHeightFloat = 300.0f;//カメラの高さ
	const Vector3 kCameraHeight = Vector3(0.0f, 150.0f, 0.0f);//カメラの高さ(Vector3)

	constexpr float kLockOnMaxDistance = 10000.0f;//ロックオンの最大距離
	constexpr float kGroundCheckDistance = 800.0f;//プレイヤーの最高到達点//カメラのターゲットの割合注視点//Y軸
	constexpr float kRatioCheckDistance = 800.0f;//プレイヤーの最高到達点//カメラのターゲットの割合注視点//XZ軸

}

LockOnCameraState::LockOnCameraState(std::weak_ptr<CameraManager> owner) : CameraStateBase(owner)
{
}

LockOnCameraState::~LockOnCameraState()
{
}

void LockOnCameraState::Enter(CameraData data)
{
	auto player = m_owner.lock()->GetContext()->m_player.lock();
	Update();
}

void LockOnCameraState::Update()
{
	//ロックオンカメラもPlayerCaemraのように
	//プレイヤーの高さに応じて注視点の割合を決めて変える

	auto cameraManager = m_owner.lock();
	if (!cameraManager)return;
	auto lockOnManager = cameraManager->GetLockOnManager().lock();
	std::shared_ptr<EnemyBase> enemy;
	if (lockOnManager) enemy = lockOnManager->GetTarget().lock();
	auto player = cameraManager->GetContext()->m_player.lock();
	auto mainCamera = cameraManager->GetMainCamera();
	if (!player)return;
	if (!mainCamera)return;
	if (!enemy)
	{
		//ターゲットがいない場合、かつ、PlayerCameraに切り替える
		cameraManager->SetNextCameraPriority(Camera::Type::PlayerCamera, false, false);
		return;
	}


	//ロックオンカメラの時、常にPlayerCameraにアングルを渡し続ける
	if (cameraManager->GetHighestPriorityCamera()->GetCameraType() == Camera::Type::LockOnCamera)
	{
		//このカメラがメインの時、PlayerCameraにangleをずっと渡す
		cameraManager->SetPlayerCameraAngle(m_angleH, m_angleV);
	}


	//目標ターゲットを計算
	FixCameraPos();
	Vector3 playerPos = player->GetPos();
	Vector3 enemyPos = enemy->GetPos();

	float distance = (enemyPos - playerPos).Magnitude();
	//ロックオンの最大距離を超えたらロックオンを解除する
	if (distance > kLockOnMaxDistance)
	{
		mainCamera->SetLockOn(false);
		//解放
		//m_lockOnEnemy.reset();

		//PlayerCameraに切り替える//処理を呼ぶ
		cameraManager->ChangeState(std::make_shared<PlayerFollowCamera>(m_owner));
	}
	Vector3 targetPos = (playerPos + enemyPos) / 2;
	//注視点の割合を決める
	float dis = (enemyPos - playerPos).Magnitude();
	//割合を決める
	float ratio = (dis - kRatioCheckDistance) / kRatioCheckDistance;
	ratio = std::clamp(ratio, 0.1f, 0.5f);
	targetPos = playerPos + (enemyPos - playerPos) * ratio;

	auto stage = m_stage.lock();
	if (stage)
	{
		Vector3 endPos = targetPos + Vector3(0.0f, -kGroundCheckDistance, 0.0f);
		//stage地面とターゲットの距離を取得する
		auto hitPoly = MV1CollCheck_Line(stage->GetStageModelHandle(), -1, targetPos.ToDxLibVector(), endPos.ToDxLibVector());
		if (hitPoly.HitFlag)
		{
			float distance = targetPos.y - hitPoly.HitPosition.y;
			if (distance >= 0.0f)
			{
				//カメラ注視点の割合をきめる
				float ratio = distance / kGroundCheckDistance;
				targetPos.y *= ratio;
			}
		}
	}


	//ターゲットの位置を更新
	m_target = targetPos + kCameraHeight;
	//カメラの位置を調整する
	//FixCameraPos();
	//カメラの位置と注視点を反映する
}

void LockOnCameraState::Exit()
{
}

void LockOnCameraState::FixCameraPos()
{
}

void LockOnCameraState::CameraSetting()
{
}
