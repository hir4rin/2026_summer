#include "Camera.h"
#include "../Math/Matrix4x4.h"
#include "EffekseerForDXLib.h"
namespace
{
	//constexpr int kNormalRx = 128;//右スティックの通常の値

	constexpr float kToPlayerLength = 700.0f;//プレイヤーからカメラまでの距離
	constexpr float kCameraHeightFloat = 300.0f;//カメラの高さ
	constexpr float kCameraAngleSpeed = 0.03f;//カメラの回転速度
	const Vector3 kCameraHeight = Vector3(0.0f, 150.0f, 0.0f);//カメラの高さ(Vector3)
}

Camera::Camera():
	m_target(0.0f, 0.0f, 0.0f),
	m_pos(0.0f, 300.0f, -700.0f)
{
	// ライトの初期化（ディレクショナルライト）
	m_lightHandle = CreateDirLightHandle(VGet(0.0f, -1.0f, 1.0f));	
	SetLightDifColorHandle(m_lightHandle, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));//ディフューズカラーを白に設定
	SetLightSpcColorHandle(m_lightHandle, GetColorF(0.5f, 0.5f, 0.5f, 1.0f));//スペキュラカラーをグレーに設定
	SetLightAmbColorHandle(m_lightHandle, GetColorF(0.3f, 0.3f, 0.3f, 1.0f));//アンビエントカラーを暗めのグレーに設定
}

Camera::~Camera()
{
	// ライトの削除
	if (m_lightHandle != -1)
	{
		DeleteLightHandle(m_lightHandle);
		m_lightHandle = -1;
	}
}

void Camera::TitleInit()
{
	m_pos = Vector3(300.0f, 120.0f, 0.0f);//カメラの位置をタイトル画面用に設定する
	m_target = Vector3(0.0f, 150.0f, 0.0f);//注視点をタイトル画面用に設定する
	// カメラの設定
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraNearFar(10.0f, 1500.0f);
}

void Camera::Init()
{
	// カメラの設定
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraNearFar(100.0f, 1500.0f);
}

void Camera::Update(Vector3 pos,Vector3 pos2)
{
	// EffekseerとDXライブラリのカメラ情報を同期する（これがないとエフェクトが描画されない）
	//Effekseer_Sync3DSetting();
}
void Camera::Draw()
{
}

void Camera::FixCameraPos()
{
	
}

void Camera::StartCameraShake(float power, float time)
{
	m_shakePower = power;
	m_shakeTimer = time;
	m_shakeTimerMax = time;
	m_isShaking = true;
}

Vector3 Camera::CameraShakeUpdate()
{
	if (m_shakeTimer <= 0.0f)
	{
		m_isShaking = false;
		return Vector3();
	}
	m_shakeTimer -= 1.0;//

	float progress = m_shakeTimer / m_shakeTimerMax;//揺れの進行度合いを0から1の範囲で表す
	float currentPower = m_shakePower * progress;//現在の揺れの強さを計算する

	float magX = (GetRand(200) / 100.0f - 1.0f) * currentPower;
	float magY = (GetRand(200) / 100.0f - 1.0f) * currentPower;
	Vector3 mag = Vector3(magX, magY, 0.0f);

	return mag;
	

}

void Camera::UpdateLight()
{
	if (m_lightHandle == -1) return;

	// カメラの向き（カメラから注視点へのベクトル）を計算
	Vector3 lightDir = (m_target - m_pos).Normalize();

	// ライトの方向を設定（カメラと同じ方向を照らす）
	SetLightDirectionHandle(m_lightHandle, lightDir.ToDxLibVector());
}

void Camera::CameraSetting()
{
	//そのフレームのカメラをセット
}

void Camera::InputRightStick()
{
	
}
