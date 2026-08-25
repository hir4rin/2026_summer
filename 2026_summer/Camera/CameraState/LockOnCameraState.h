#pragma once
#include "CameraStateBase.h"



class LockOnCameraState : public CameraStateBase
{
public:
	LockOnCameraState(std::weak_ptr<CameraManager> owner);
	virtual ~LockOnCameraState();
	virtual void Enter(CameraData data)override;
	virtual void Update()override;
	virtual void Exit()override;

	void FixCameraPos() override;
	void CameraSetting() override;
};

