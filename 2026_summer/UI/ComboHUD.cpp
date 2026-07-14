#include "ComboHUD.h"
#include "../Game.h"


namespace
{
	constexpr int kButtonWidth = 128;
	constexpr int kButtonHeight = 128;
}

ComboHUD::ComboHUD()
{
	m_buttonHandles.resize(static_cast<int>(ComboButton::ButtonSize));
	//ハンドルの読み込み、リサイズ
	Init();
	
}

ComboHUD::~ComboHUD()
{
}

void ComboHUD::Init()
{
	m_buttonHandles[static_cast<int>(ComboButton::X)] = MV1LoadModel("data/UI/button/X.mv1");
	m_buttonHandles[static_cast<int>(ComboButton::Y)] = MV1LoadModel("data/UI/button/Y.mv1");
	m_buttonHandles[static_cast<int>(ComboButton::A)] = MV1LoadModel("data/UI/button/A.mv1");
	m_buttonHandles[static_cast<int>(ComboButton::B)] = MV1LoadModel("data/UI/button/B.mv1");
	m_buttonHandles[static_cast<int>(ComboButton::RB)] = MV1LoadModel("data/UI/button/RB.mv1");
	m_buttonHandles[static_cast<int>(ComboButton::RT)] = MV1LoadModel("data/UI/button/RT.mv1");
	m_buttonHandles[static_cast<int>(ComboButton::LB)] = MV1LoadModel("data/UI/button/LB.mv1");
	m_buttonHandles[static_cast<int>(ComboButton::LT)] = MV1LoadModel("data/UI/button/LT.mv1");

}

void ComboHUD::Update()
{
}

void ComboHUD::Draw() const
{
	//ここで、コンボのボタンを描画する処理を実装する
	
	//弱攻撃のコンボボタン
	for(int i = 0; i < 5; ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kCombo1[i])];
		DrawRectRotaGraph(ComboUI::kCombo1StartX + i * 50, ComboUI::kCombo1StartY,0,0, kButtonWidth, kButtonHeight,1.0f,0.0f, handle, true);
	}
	//強攻撃のコンボボタン
	for(int i = 0; i < 2; ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kCombo2[i])];
		DrawRectRotaGraph(ComboUI::kCombo2StartX + i * 50, ComboUI::kCombo2StartY,0,0, kButtonWidth, kButtonHeight,1.0f,0.0f, handle, true);
	}

}
