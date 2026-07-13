#pragma once
#include "../Math/Vector3.h"
class UIBase
{
public:
	UIBase() = default;
	virtual ~UIBase() = default;
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() const = 0;
protected:
	Vector3 m_pos = {};
	Vector3 m_vel = {};
	float m_angle = 0.0f;
	float m_scale = 1.0f;
};

