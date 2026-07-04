#pragma once
#include "Camera.h"

class LockOnCamera : public Camera
{
public:
	LockOnCamera();
	~LockOnCamera();
	void Init();
	void Update(Vector3 pos, Vector3 pos2 = Vector3());
	void FixCameraPos() override;
	void CameraSetting() override;
	void Draw() override;
	void SetCameraData(const CameraData& data) override { m_cameraData = data; Init(); }
private:

};

