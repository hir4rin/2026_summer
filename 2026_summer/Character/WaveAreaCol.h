#pragma once
#include "Collider.h"
class WaveAreaCol :
    public Collider
{
public :
    WaveAreaCol();
    virtual ~WaveAreaCol() = default;
	void OnCollision(Collider& other) override;
	void ApplyPos() override;

	void Draw();
private:
};

