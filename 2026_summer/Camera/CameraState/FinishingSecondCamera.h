#pragma once
#include "CameraStateBase.h"

//ボス撃破後の演出用カメラ(2つ目:血殺を打つ瞬間を映す)
class FinishingSecondCamera : public CameraStateBase
{
public:
	FinishingSecondCamera(std::weak_ptr<CameraManager> owner);
	virtual ~FinishingSecondCamera();
	virtual void Enter(CameraData data)override;
	virtual void Update()override;
	virtual void Exit()override;

	void FixCameraPos() override;
	void CameraSetting() override;

	Type GetCameraType()const override { return Type::FinishingSecondCamera; }
	virtual BlendSetting GetBlendSetting()const override
	{
		return BlendSetting{
			.mode = BlendSetting::Mode::Lerp,
			.duration = 40.0f,
			.easingPower = 0.4f,
			.pivot = Vector3()
		};
	}
	BlendSetting GetOverrideBlendSetting()const override
	{
		return BlendSetting{
		.mode = BlendSetting::Mode::Lerp,
		.duration = 30.0f,
		.easingPower = 1.0f,
		.pivot = Vector3()

		};
	}
private:
	float m_timer = 0.0f;//演出開始からの経過フレーム//一定フレームで元のStateに戻すのに使う
};
