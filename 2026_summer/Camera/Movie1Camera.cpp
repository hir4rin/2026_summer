#include "Movie1Camera.h"
#include "../Math/Matrix4x4.h"
#include <cmath>

namespace
{
	//constexpr int kNormalRx = 128;//右スティックの通常の値

	constexpr float kToPlayerLength = 400.0f;//プレイヤーからカメラまでの距離
	constexpr float kCameraHeightFloat = 300.0f;//カメラの高さ
	constexpr float kCameraAngleSpeed = 0.03f;//カメラの回転速度
	const Vector3 kCameraHeight = Vector3(0.0f, 150.0f, 0.0f);//カメラの高さ(Vector3)

	constexpr float kInvisibleFrame = 50.0f;//ムービーカメラを優先するフレーム数//2秒間はムービーカメラを優先するようにするためのもの//60fpsの場合
}


Movie1Camera::Movie1Camera():
	m_invisibleFrame(kInvisibleFrame)
{
	m_priority = 0;//ムービーカメラは優先度が低い
	m_pos = Vector3(0.0f, kCameraHeightFloat, -kToPlayerLength);
	m_target = Vector3(0.0f, 0.0f, 0.0f);
	m_type = Type::Movie1Camera;
}

Movie1Camera::~Movie1Camera()
{
}

void Movie1Camera::Init()
{
	// カメラの設定
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraNearFar(0.0f, 1500.0f);
}

void Movie1Camera::Update(Vector3 pos, Vector3 pos2)
{
	//priorityがゼロの時はこうしんしない処理
	if(m_priority != 0)CountDiePriority();
	//ターゲットの位置を更新
	Vector3 playerPos = pos;
	playerPos.y = 0.0f;//プレイヤーのy座標は0にする
	m_target = playerPos + kCameraHeight;

	//カメラの位置を調整する
	FixCameraPos();
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void Movie1Camera::FixCameraPos()
{
	//水平方向の回転
	auto rotY = Matrix4x4::MakeRotationY(m_angleH);
	m_angleV = 0.06f;//垂直方向の回転角度は固定する
	auto rotX = Matrix4x4::MakeRotationX(m_angleV);

	float cameraToPlayerLength = kToPlayerLength * 0.5f;

	//カメラの座標を算出
	auto CtoP = Vector3(0.0f, 0.0f, -cameraToPlayerLength);//プレイヤーからカメラへのベクトル
	//DxLibに変換
	auto CtoPVec = CtoP.ToDxLibVector();
	auto rotYMat = rotY.ToDxLibMatrix(rotY);//回転行列を転置する
	auto rotXMat = rotX.ToDxLibMatrix(rotX);//回転行列を転置する

	auto RotCtoP = VTransform(CtoPVec, rotXMat);//回転させる
	RotCtoP = VTransform(RotCtoP, rotYMat);//回転させる

	auto pos = VAdd(RotCtoP, m_target.ToDxLibVector());//プレイヤーの座標に足す

	m_pos = Vector3::FromDxLibVector(pos);

	//posからtargetへのベクトルを求める
	auto toTarget = m_target - m_pos;
	toTarget = toTarget.Normalize();
	//少し先にカメラ到達地点を設定する
	m_nextpos = m_pos + toTarget * 10.0f;
	//カメラの位置を補完
	m_pos.x = std::lerp(m_pos.ToDxLibVector().x, m_nextpos.ToDxLibVector().x, 0.4f);//カメラの位置を滑らかに移動させる
	m_pos.y = std::lerp(m_pos.ToDxLibVector().y, m_nextpos.ToDxLibVector().y, 0.4f);//カメラの位置を滑らかに移動させる
	m_pos.z = std::lerp(m_pos.ToDxLibVector().z, m_nextpos.ToDxLibVector().z, 0.4f);//カメラの位置を滑らかに移動させる

}

void Movie1Camera::CountDiePriority()
{
	m_invisibleFrame--;
	if (m_invisibleFrame < 0.0f)
	{
		m_invisibleFrame = kInvisibleFrame;
		m_isChangeCamera = true;
	}
}
