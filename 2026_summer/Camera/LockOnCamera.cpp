#include "LockOnCamera.h"
#include "CameraManager.h"
#include "../Character/Enemy/EnemyBase.h"
#include "Player.h"
#include "MainCamera.h"
#include "../Math/Matrix4x4.h"
namespace
{
	constexpr float kToPlayerLength = 350.0f;//プレイヤーからカメラまでの距離
	constexpr float kCameraHeightFloat = 300.0f;//カメラの高さ
	const Vector3 kCameraHeight = Vector3(0.0f, 150.0f, 0.0f);//カメラの高さ(Vector3)

	constexpr float kLockOnMaxDistance = 10000.0f;//ロックオンの最大距離
}

LockOnCamera::LockOnCamera()
{
	//変数をすべて初期化//一旦
	m_type = Type::LockOnCamera;
	m_priority = 0;
	m_pos = Vector3(0.0f, 0.0f, 0.0f);//いらないと思うけどなぜか正規化
	m_target = Vector3(0.0f, 0.0f, 0.0f);
}

LockOnCamera::~LockOnCamera()
{
}

void LockOnCamera::Init()
{
}

void LockOnCamera::Update(Vector3 pos, Vector3 pos2)
{
	auto enemy = m_cameraContext->m_targetEnemy.lock();
	auto player = m_cameraContext->m_player.lock();
	auto cameraManager = m_cameraManager.lock();
	auto mainCamera = cameraManager->GetMainCamera();
	if (!enemy)return;
	if (!player)return;
	if (!mainCamera)return;

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

	float distance = (enemyPos - pos).Magnitude();
	//ロックオンの最大距離を超えたらロックオンを解除する
	if (distance > kLockOnMaxDistance)
	{
		mainCamera->SetLockOn(false);
		//解放
		//m_lockOnEnemy.reset();


		//PlayerCameraに切り替える
		auto cameraManager = m_cameraManager.lock();
		if (!cameraManager)return;
		cameraManager->SetNextCameraPriority(Camera::Type::PlayerCamera, false, true);
	}

	//ターゲットの位置を更新
	m_target = (playerPos + enemyPos) / 2 + kCameraHeight;
	//カメラの位置を調整する
	FixCameraPos();
	//カメラの位置と注視点を反映する

}

void LockOnCamera::FixCameraPos()
{
	auto enemy = m_cameraContext->m_targetEnemy.lock();
	auto player = m_cameraContext->m_player.lock();
	if (!enemy)return;
	if (!player)return;

	//水平方向の回転//敵との距離によってこの角度を帰る
	auto rotY = Matrix4x4::MakeRotationY(0.2f);
	//auto rotX = Matrix4x4::MakeRotationX(0.16f);//固定

	//ここも敵との距離に寄って変える
	float cameraToPlayerLength = kToPlayerLength;

	//カメラの座標を算出
	//PtoEVecの逆ベクトルを15度程度ずらす、playerの座標から足す
	Vector3 playerPos = player->GetPos();
	Vector3 enemyPos = enemy->GetPos();
	//y座標を0にする
	playerPos.y = enemyPos.y = 0.0f;

	Vector3 EtoPVec = (playerPos - enemyPos).Normalize();
	EtoPVec *= cameraToPlayerLength;


	//auto CtoP = Vector3(0.0f, 0.0f, -cameraToPlayerLength);//プレイヤーからカメラへのベクトル

	//カメラの揺れを加える
	//EtoPVec = EtoPVec + CameraShakeUpdate();
	//DxLibに変換
	auto EtoPVecDx = EtoPVec.ToDxLibVector();
	auto rotYMat = Matrix4x4::ToDxLibMatrix(rotY);//回転行列を転置する
	//auto rotXMat = Matrix4x4::ToDxLibMatrix(rotX);//回転行列を転置する

	//カメラからプレイヤーVec
	auto RotPtoC = VTransform(EtoPVecDx, rotYMat);//回転させる
	//RotPtoC = VTransform(RotPtoC, rotXMat);//回転させる
	RotPtoC = VAdd(RotPtoC, VGet(0.0f, 160.0f, 0.0f));

	//水平方向はその向き、垂直は初期化で角度を更新し、プレイヤーカメラに渡す
	//atan2f(cross,dot)で二つのベクトルの角度が出る//理解済み
	//Vector3 BaseVec = Vector3(0.0f, 0.0f, 1.0f);//Z軸の正方向//これを基準にする
	//float dot = BaseVec.Dot(RotPtoC);
	// float cross = BaseVec.Cross2DXZ(Vector3::FromDxLibVector(RotPtoC));
	//m_angleH = atan2f(cross, dot);
	//m_angleH += DX_PI_F;
	m_angleH = atan2f(RotPtoC.x, RotPtoC.z) + DX_PI_F;


	auto pos = VAdd(RotPtoC, player->GetPos().ToDxLibVector());//プレイヤーの座標に足す

	m_pos = Vector3::FromDxLibVector(pos);
}

void LockOnCamera::CameraSetting()
{
}

void LockOnCamera::Draw()
{
	auto cameraManager = m_cameraManager.lock();
	auto enemy = m_cameraContext->m_targetEnemy.lock();
	auto player = m_cameraContext->m_player.lock();
	if (!enemy)return;
	if (!player)return;

	DrawSphere3D(m_pos.ToDxLibVector(), 10.0f, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), true);
	//このカメラがメインカメラの時、デバッグ
	if (cameraManager->GetHighestPriorityCamera()->GetCameraType() == Camera::Type::LockOnCamera)
	{
		Vector3 startPos = player->GetPos() + Vector3(0,100,0);
		Vector3 endPos = enemy->GetPos() + Vector3(0,100,0);
		DrawLine3D(startPos.ToDxLibVector(), endPos.ToDxLibVector(), GetColor(0, 0, 255));
	}
}
