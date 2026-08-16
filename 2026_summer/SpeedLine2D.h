#pragma once
#include <vector>

struct LineData
{
	float x;
	float y;
	float length;
	int life;
	int maxLife;
	float speed;
};

class SpeedLine2D
{
public:
	SpeedLine2D();
	virtual ~SpeedLine2D();

	void TryGenerate(bool isMoving);
	void Update();
	void Draw();
private:
	std::vector<LineData> m_lines;
	int m_generateInterval = 0;
};

