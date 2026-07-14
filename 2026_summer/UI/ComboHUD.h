#pragma once
#include "UIBase.h"
#include <memory>
#include <vector>

enum class ComboButton : int
{
	X = 0,
	Y = 1,
	A = 2,
	B = 3,
	RB = 4,
	RT = 5,
	LB = 6,
	LT = 7,
	ButtonSize = 8

};

class ComboHUD :
    public UIBase
{
public:
    ComboHUD();
    ~ComboHUD();

    void Init() override;
    void Update() override;
	void Draw() const override;
private:
    //モデルハンドル
	std::vector<int> m_buttonHandles;


};

