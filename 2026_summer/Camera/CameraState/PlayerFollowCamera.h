#pragma once
#include "CameraStateBase.h"
class PlayerFollowCamera :public CameraStateBase
{
public:
	PlayerFollowCamera(std::weak_ptr<CameraManager> owner);
	virtual ~PlayerFollowCamera();
	virtual void Enter(CameraData data)override;
	virtual void Update()override;
	virtual void Exit()override;

	void FixCameraPos() override;
	void CameraSetting() override;

private:
	void InputRightStick();//右スティックの入力を処理する
	void FixCameraPosLockOn();//ロックオン時のカメラの位置を調整する
private:
	XINPUT_STATE  xi;
	Vector3 m_rayVec = Vector3(0, 0, 0);//カメラの前方向のベクトル//カメラの注視点を決めるために使う　
	//lerp用
	Vector3 m_targetPos;//カメラの目標座標
	Vector3 m_focusGoal;//カメラの目標注視点座標

	Vector3 m_testPos = Vector3(0, 0, 0);
	Vector3 m_testPos2 = Vector3(0, 0, 0);
};

