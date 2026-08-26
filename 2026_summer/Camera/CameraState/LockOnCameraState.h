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

	Type GetCameraType()const override { return Type::LockOnCamera; }
	virtual BlendSetting GetBlendSetting()const override
	{
		return BlendSetting{
			.mode = BlendSetting::Mode::Lerp,
			.duration = 25.0f,
			.easingPower = 0.3f,
			.pivot = Vector3()
		};
	}

};

