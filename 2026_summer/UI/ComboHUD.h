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
	RStickLR = 8,
	Hold = 9,
	ButtonSize = 10

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

	void ToggleVisible() { m_isVisible = !m_isVisible; }//表示のオン/オフを切り替える
	bool IsVisible() const { return m_isVisible; }
private:
	//設計解像度の座標を画面比率の座標に変換する(文字表示用)
	int SX(int designX) const;
	int SY(int designY) const;

	//コンボボタンを描画する(hold.pngは450x400と他のボタン画像より大きいため、実寸法をソース矩形にして、他のボタンと同じくらいの大きさに縮小する)
	void DrawComboButton(int x, int y, int buttonIndex, int handle, double scale) const;

    //モデルハンドル
	std::vector<int> m_buttonHandles;

	bool m_isVisible = true;//コンボUIを表示するかどうか

};

