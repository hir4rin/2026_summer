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
	m_buttonHandles[static_cast<int>(ComboButton::X)] = LoadGraph("data/UI/button/X.png");
	m_buttonHandles[static_cast<int>(ComboButton::Y)] = LoadGraph("data/UI/button/Y.png");
	m_buttonHandles[static_cast<int>(ComboButton::A)] = LoadGraph("data/UI/button/A.png");
	m_buttonHandles[static_cast<int>(ComboButton::B)] = LoadGraph("data/UI/button/B.png");
	m_buttonHandles[static_cast<int>(ComboButton::RB)] = LoadGraph("data/UI/button/RB.png");
	m_buttonHandles[static_cast<int>(ComboButton::RT)] = LoadGraph("data/UI/button/RT.png");
	m_buttonHandles[static_cast<int>(ComboButton::LB)] = LoadGraph("data/UI/button/LB.png");
	m_buttonHandles[static_cast<int>(ComboButton::LT)] = LoadGraph("data/UI/button/LT.png");

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
		DrawRectRotaGraph(ComboUI::kCombo1StartX + i * ComboUI::ButtonToButtonDistanceX, ComboUI::kCombo1StartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
	}
	//強攻撃のコンボボタン
	for(int i = 0; i < 2; ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kCombo2[i])];
		DrawRectRotaGraph(ComboUI::kCombo2StartX + i * ComboUI::ButtonToButtonDistanceX, ComboUI::kCombo2StartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
	}
	//スキル攻撃のコンボボタン
	for(int i = 0; i < 3; ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kSkillCombo[i])];
		DrawRectRotaGraph(ComboUI::kSkillStartX + i * ComboUI::ButtonToButtonDistanceX, ComboUI::kSkillStartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
	}
	//必殺技のボタン
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kUlt[0])];
		DrawRectRotaGraph(ComboUI::kUltStartX, ComboUI::kUltStartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
	}
	//ロックオン
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kLockOn)];
		DrawRectRotaGraph(ComboUI::kLockOnStartX, ComboUI::kLockOnStartY, 0, 0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale, 0.0f, handle, true);
	}


}
