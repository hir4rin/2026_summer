#include "UltCamera.h"
#include "../Math/Matrix4x4.h"
#include "Player.h"
#include "../System.h"
#include "../Character/Enemy/EnemyBase.h"
#include "../Camera/CameraManager.h"

namespace
{
	constexpr float kToPlayerLength = 700.0f;//プレイヤーからカメラまでの距離
	constexpr float kCameraHeightFloat = 300.0f;//カメラの高さ
	const Vector3 kCameraHeight = Vector3(0.0f, 150.0f, 0.0f);//カメラの高さ(Vector3)

	const float kUltDistance = kToPlayerLength * 4.0f;
}

UltCamera::UltCamera()
{
	m_type = Type::UltCamera;
	m_priority = 0;
	m_pos = Vector3(0.0f, 0.0f, 0.0f);//いらないと思うけどなぜか正規化
	m_target = Vector3(0.0f, 0.0f, 0.0f);
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
	
	//目標ターゲットを計算
	FixCameraPos();
	//まず回転角度をlerpする
	Vector3 playerPos = pos;
	Vector3 enemyPos = pos2;
	Vector3 cameraPos = m_pos;
	playerPos.y = enemyPos.y = cameraPos.y =  0.0f;
	Vector3 PtoCVec = (cameraPos - playerPos).Normalize();
	//playerからtargetPosへのベクトルを正規化
	Vector3 TargetVecNorm = m_targetPos - playerPos;
	TargetVecNorm.y = 0.0f;
	TargetVecNorm = TargetVecNorm.Normalize();
	//ここをSlerpする//2次元上での回転のみ
	PtoCVec = Vector3::Slerp(PtoCVec, TargetVecNorm, 0.1f);
	//ベクトルの大きさをlerpする
	m_VecLength = std::lerp(m_VecLength, kUltDistance, 0.1f);
	//大きさをたす
	PtoCVec *= m_VecLength;
	//高さを足す
	PtoCVec += kCameraHeight;
	//カメラの座標を更新
	m_pos = playerPos + PtoCVec;

	//注視点
	m_target = (playerPos + enemyPos) / 2;
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());

	//pos1からpos2のほうに向かって、中心点(m_target)から円状にカメラを回転させる
	//距離も近くする
	//m_pos = Vector3::Lerp(m_pos, m_targetPos, 0.01f);

	bool isUlt = System::GetInstance().GetIsUltimating();
	if (!isUlt)
	{
		//priorityを下げる
		auto cameraManager = m_cameraManager.lock();
		if (!cameraManager)return;
		cameraManager->SetAllCameraPriority(Camera::Type::PlayerCamera);
	}
	

}

void UltCamera::FixCameraPos()
{


	auto enemy = m_lockOnEnemy.lock();
	auto player = m_player.lock();
	if (!enemy)return;
	if (!player)return;

	//水平方向の回転//敵との距離によってこの角度を帰る
	auto rotY = Matrix4x4::MakeRotationY(0.6f);
	//auto rotX = Matrix4x4::MakeRotationX(0.16f);//固定

	//固定//そのまま
	Vector3 PtoCVec = (m_pos - player->GetPos());
	float cameraToPlayerLength = PtoCVec.Magnitude();
	m_VecLength = cameraToPlayerLength;

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
	//水平方向はその向き、垂直は初期化
	m_angleH = atan2f(RotPtoC.x, RotPtoC.z) + DX_PI_F;
	m_angleV;

	auto pos = VAdd(RotPtoC, player->GetPos().ToDxLibVector());//プレイヤーの座標に足す

	m_targetPos = Vector3::FromDxLibVector(pos);
}

void UltCamera::CameraSetting()
{
	//FixCameraPos();
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void UltCamera::Draw()
{
}
