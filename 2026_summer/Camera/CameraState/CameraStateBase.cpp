#include "CameraStateBase.h"


CameraStateBase::CameraStateBase(std::weak_ptr<CameraManager> owner):
	m_owner(owner)
{
}

CameraStateBase::~CameraStateBase()
{
}

void CameraStateBase::Draw()
{
}

void CameraStateBase::FixCameraPos()
{

}

void CameraStateBase::StartCameraShake(float power, float time)
{
	m_shakePower = power;
	m_shakeTimer = time;
	m_shakeTimerMax = time;
	m_isShaking = true;
}

Vector3 CameraStateBase::CameraShakeUpdate()
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

void CameraStateBase::CameraSetting()
{
	//そのフレームのカメラをセット
}

