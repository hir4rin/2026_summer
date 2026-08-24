#include "UltCamera.h"
#include "../Math/Matrix4x4.h"
#include "Player.h"
#include "../System.h"
#include "../Character/Enemy/EnemyBase.h"
#include "../Camera/CameraManager.h"
#include "../Camera/LockOnManager.h"
#include "../SubWindow/SubWindow.h"
#include "MainCamera.h"
#include <algorithm>
namespace
{
	constexpr float kToPlayerLength = 700.0f;//プレイヤーからカメラまでの距離
	constexpr float kCameraHeightFloat = 300.0f;//カメラの高さ
	const Vector3 kCameraHeight = Vector3(0.0f, 150.0f, 0.0f);//カメラの高さ(Vector3)

	const float kUltDistance = kToPlayerLength   / 2.0f;
	//const float kUltDistance = kToPlayerLength   * 2.0f;
	constexpr float kRatioCheckDistance = 800.0f;//プレイヤーの最高到達点//カメラのターゲットの割合注視点//XZ軸
}

UltCamera::UltCamera()
{
	//変数をすべて初期化//一旦
	m_type = Type::UltCamera;
	m_priority = 0;
	m_pos = Vector3(0.0f, 0.0f, 0.0f);//いらないと思うけどなぜか正規化
	m_target = Vector3(0.0f, 0.0f, 0.0f);
	m_cameraSetUp = { .DoLerp = false, .DoSlerp = true };
}

UltCamera::~UltCamera()
{
}

void UltCamera::Init()
{
	//結局playerとenemyをweak_ptrで持ったのでたぶんつかわない

	//目標点などの初期化
	m_pos = m_cameraData.pos;
	m_target = m_cameraData.target;
	float angleH = m_cameraData.angleH;
	//初期化
	m_VecLength = 0.0f;
	m_targetPos;

	//ここでカメラアングルの場合によって演出の向きを変えるようにしたい
}

void UltCamera::Update(Vector3 pos, Vector3 pos2)
{
	//やりたいこと
	// EtoPVecをプレイヤーにたした今の座標から
	// そのベクトルをplayerPos中心で一定角度になるまで回す&&距離を縮めた値へlerpしていきたい
	//
	//Playerの後ろへカメラが行くベクトルがいい→春休みの作品を参考に
	//あと、カメラハンドラーのほうでカメラを作り直す
	//そもそもEnemyとPlayerのポインタを受け取っていなかった
	

	auto cameraManager = m_cameraManager.lock();
	if (!cameraManager)return;
	auto lockOnManager = cameraManager->GetLockOnManager().lock();
	std::shared_ptr<EnemyBase> enemy;
	if (lockOnManager) enemy = lockOnManager->GetTarget().lock();
	auto player = m_cameraContext->m_player.lock();
	if (!enemy)
	{
		//敵がいない場合は、PlayerCameraに切り替える//このカメラが一番優先度高いとき
		if (cameraManager->GetHighestPriorityCamera()->GetCameraType() != Camera::Type::UltCamera)return;
		cameraManager->SetNextCameraPriority(Camera::Type::PlayerCamera, true, false);
		return;
	}
	if (!player)return;
	//ウルトカメラ時、常にPlayerCameraにアングルを渡し続ける
	if (cameraManager->GetHighestPriorityCamera()->GetCameraType() == Camera::Type::UltCamera)
	{
		//このカメラがメインの時、PlayerCameraにangleをずっと渡す
		cameraManager->SetPlayerCameraAngle(m_angleH, m_angleV);
	}



	//目標ターゲットを計算
	FixCameraPos();
	//まず回転角度をlerpする
	Vector3 playerPos = player->GetPos();
	Vector3 enemyPos = enemy->GetPos();
	//Vector3 cameraPos = m_pos;
	//playerPos.y = enemyPos.y = cameraPos.y =  0.0f;
	//Vector3 PtoCVec = (cameraPos - playerPos).Normalize();
	////playerからtargetPosへのベクトルを正規化
	//Vector3 TargetVecNorm = m_targetPos - playerPos;
	//TargetVecNorm.y = 0.0f;
	//TargetVecNorm = TargetVecNorm.Normalize();
	////ここをSlerpする//2次元上での回転のみ
	////PtoCVec = Vector3::Slerp(PtoCVec, TargetVecNorm, 0.1f);
	//PtoCVec = TargetVecNorm;
	////ベクトルの大きさをlerpする
	////m_VecLength = std::lerp(m_VecLength, kUltDistance, 0.1f);
	//m_VecLength = kUltDistance;
	////大きさをたす
	//PtoCVec *= m_VecLength;
	////高さを足す
	//PtoCVec += kCameraHeight;
	////カメラの座標を更新
	//m_pos = playerPos + PtoCVec;

	//注視点
	m_target = (playerPos + enemyPos) / 2 + kCameraHeight;
	//注視点の割合を決める//あんまり気に入ってない//なんか変だから値を小さくしている
	enemyPos.y = playerPos.y = 0.0f;
	float dis = (enemyPos - playerPos).Magnitude();
	//割合を決める
	float ratio = (dis - kRatioCheckDistance) / kRatioCheckDistance;
	ratio = std::clamp(ratio, 0.5f, 0.6f);
	m_target = playerPos + (enemyPos - playerPos) * ratio + kCameraHeight;

	//カメラの位置と注視点を反映する
	//SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());

	//pos1からpos2のほうに向かって、中心点(m_target)から円状にカメラを回転させる
	//距離も近くする
	//m_pos = Vector3::Lerp(m_pos, m_targetPos, 0.01f);

	//このカメラが一番優先度が高いときにウルトがfalseになったら、PlayerCameraに切り替える
	auto isHighest = cameraManager->GetHighestPriorityCamera();
	bool isMainCamera = isHighest->GetCameraType() == Camera::Type::UltCamera;
	bool isUlt = System::GetInstance().GetIsUltimating();
	if (!isUlt && isMainCamera)
	{
		bool isLockOn = cameraManager->GetMainCamera()->GetIsLockOn();
		//priorityを下げる
		if (isLockOn)
		{
			cameraManager->SetNextCameraPriority(Camera::Type::LockOnCamera, false, true);
		}
		else
		{
			cameraManager->SetNextCameraPriority(Camera::Type::PlayerCamera, false, true);
		}
	}
}

void UltCamera::FixCameraPos()
{
	auto cameraManager = m_cameraManager.lock();
	if (!cameraManager)return;
	auto lockOnManager = cameraManager->GetLockOnManager().lock();
	std::shared_ptr<EnemyBase> enemy;
	if (lockOnManager) enemy = lockOnManager->GetTarget().lock();
	auto player = m_cameraContext->m_player.lock();
	auto mainCamera = cameraManager->GetMainCamera();
	if (!enemy)return;
	if (!player)return;

	//auto rotX = Matrix4x4::MakeRotationX(0.16f);//固定

	//固定//そのまま
	//Vector3 PtoCVec = (m_pos - player->GetPos());
	//float cameraToPlayerLength = PtoCVec.Magnitude();
	//m_VecLength = cameraToPlayerLength;

	//カメラの座標を算出
	//PtoEVecの逆ベクトルを15度程度ずらす、playerの座標から足す
	Vector3 playerPos = player->GetPos();
	Vector3 enemyPos = enemy->GetPos();
	//y座標を0にする
	playerPos.y = enemyPos.y = 0.0f;

	Vector3 EtoPVec = (playerPos - enemyPos).Normalize();
	EtoPVec *= 300.0f;
	//EtoPVecを90度回転させたベクトルとMainCtoPVecの内積が正か負かでどちらに回転させるかを決める
	Vector3 upVec = Vector3(0.0f, 1.0f, 0.0f );
	Vector3 rotateBase = EtoPVec.Cross(upVec).Normalize();
	Vector3 PtoMainCVec = (mainCamera->GetCameraPos() - playerPos).Normalize();
	float dot = rotateBase.Dot(PtoMainCVec);
	float angle = 0.0f;
	if (dot >= 0.0f)
	{
		angle = -DX_PI_F / 2.0f;
	}
	else
	{
		angle = DX_PI_F / 2.0f;
	}

	//水平方向の回転//敵との距離によってこの角度を帰る
	auto rotY = Matrix4x4::MakeRotationY(angle);
	
	//カメラの揺れを加える
	//EtoPVec = EtoPVec + CameraShakeUpdate();
	//DxLibに変換
	auto EtoPVecDx = EtoPVec.ToDxLibVector();
	auto rotYMat = Matrix4x4::ToDxLibMatrix(rotY);//回転行列を転置する
	//auto rotXMat = Matrix4x4::ToDxLibMatrix(rotX);//回転行列を転置する

	//カメラからプレイヤーVec
	auto RotPtoC = VTransform(EtoPVecDx, rotYMat);//回転させる
	//RotPtoC = VTransform(RotPtoC, rotXMat);//回転させる
	RotPtoC = VAdd(RotPtoC, kCameraHeight.ToDxLibVector());
	//水平方向はその向き、垂直は初期化
	m_angleH = atan2f(RotPtoC.x, RotPtoC.z) + DX_PI_F;
	//m_angleV;


	auto pos = VAdd(RotPtoC, player->GetPos().ToDxLibVector());//プレイヤーの座標に足す

	m_pos = Vector3::FromDxLibVector(pos);
}

void UltCamera::CameraSetting()
{
	//FixCameraPos();
	//カメラの位置と注視点を反映する
	//SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void UltCamera::Draw()
{
	DrawSphere3D(m_pos.ToDxLibVector(), 10.0f,16, GetColor(255, 255, 255),GetColor(0, 0, 0), true);

	std::string posText = "UltCameraPos:(" + std::to_string(m_pos.x) + "," + std::to_string(m_pos.y) + "," + std::to_string(m_pos.z) + ")";
	std::string targetText = "UltCameraTarget:(" + std::to_string(m_target.x) + "," + std::to_string(m_target.y) + "," + std::to_string(m_target.z) + ")";
	SubWindow::AddText(posText);
	SubWindow::AddText(targetText);
}
