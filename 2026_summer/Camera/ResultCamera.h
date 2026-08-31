#pragma once
#include "Camera.h"
class ResultCamera :
    public Camera
{
public:
	ResultCamera();
	virtual ~ResultCamera();

	void Init()override;
	void Draw()override;
	void Update(Vector3 pos, Vector3 pos2 = Vector3()) override;
	void CameraSetting() override;
};

