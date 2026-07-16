#include "ComboHUD.h"
#include "../Game.h"


namespace
{
	constexpr int kButtonWidth = 64;
	constexpr int kButtonHeight = 64;


	constexpr int kHoldDis = 30;
	constexpr int kHoldDisY = 5;
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
	m_buttonHandles[static_cast<int>(ComboButton::RStickLR)] = LoadGraph("data/UI/button/RStickLR.png");

}

void ComboHUD::Update()
{
}

void ComboHUD::Draw() const
{
	//ここで、コンボのボタンを描画する処理を実装する
	
	//弱攻撃のコンボボタン
	DrawFormatString(ComboUI::kCombo1StartX, ComboUI::kCombo1StartY - ComboUI::kStringDistanceY, GetColor(0, 0, 0), "弱攻撃コンボ");
	for(int i = 0; i < std::size(ComboUI::kCombo1); ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kCombo1[i])];
		DrawRectRotaGraph(ComboUI::kCombo1StartX + i * ComboUI::ButtonToButtonDistanceX, ComboUI::kCombo1StartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
	}
	//強攻撃のコンボボタン
	DrawFormatString(ComboUI::kCombo2StartX, ComboUI::kCombo2StartY - ComboUI::kStringDistanceY, GetColor(0, 0, 0), "強攻撃コンボ");
	for(int i = 0; i < std::size(ComboUI::kCombo2); ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kCombo2[i])];
		DrawRectRotaGraph(ComboUI::kCombo2StartX + i * ComboUI::ButtonToButtonDistanceX, ComboUI::kCombo2StartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
	}
	//打ち上げコンボボタン
	DrawFormatString(ComboUI::kAirComboStartX, ComboUI::kAirComboStartY - ComboUI::kStringDistanceY, GetColor(0, 0, 0), "打ち上げコンボ");
	for(int i = 0; i < std::size(ComboUI::kAirCombo); ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kAirCombo[i])];
		DrawRectRotaGraph(ComboUI::kAirComboStartX + i * ComboUI::ButtonToButtonDistanceX, ComboUI::kAirComboStartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
	}

	//スキル攻撃のコンボボタン
	DrawFormatString(ComboUI::kSkillStartX, ComboUI::kSkillStartY - ComboUI::kStringDistanceY, GetColor(0, 0, 0), "スキル攻撃コンボ");
	for(int i = 0; i < std::size(ComboUI::kSkillCombo); ++i)
	{
		//hold表記
		if (static_cast<int>(ComboUI::kSkillCombo[i]) == static_cast<int>(ComboButton::Hold))
		{
			DrawFormatString(ComboUI::kSkillStartX + i * ComboUI::ButtonToButtonDistanceX- kHoldDis, ComboUI::kSkillStartY- kHoldDisY, GetColor(0, 0, 0), "(Hold)");
			continue;
		}
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kSkillCombo[i])];
		DrawRectRotaGraph(ComboUI::kSkillStartX + i * ComboUI::ButtonToButtonDistanceX, ComboUI::kSkillStartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
	}
	//必殺技のボタン
	DrawFormatString(ComboUI::kUltStartX, ComboUI::kUltStartY - ComboUI::kStringDistanceY, GetColor(0, 0, 0), "必殺技コンボ");
	{
		for(int i = 0; i < std::size(ComboUI::kUlt); ++i)
		{
			//hold表記
			if (static_cast<int>(ComboUI::kUlt[i]) == static_cast<int>(ComboButton::Hold))
			{
				DrawFormatString(ComboUI::kUltStartX + i * ComboUI::ButtonToButtonDistanceX- kHoldDis, ComboUI::kUltStartY- kHoldDisY, GetColor(0, 0, 0), "(Hold)");
				continue;
			}
			int handle = m_buttonHandles[static_cast<int>(ComboUI::kUlt[i])];
			DrawRectRotaGraph(ComboUI::kUltStartX + i * ComboUI::ButtonToButtonDistanceX, ComboUI::kUltStartY,0,0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale,0.0f, handle, true);
		}
	}
	//ロックオン
	DrawFormatString(ComboUI::kLockOnStartX, ComboUI::kLockOnStartY - ComboUI::kStringDistanceY, GetColor(0, 0, 0), "ロックオン");
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kLockOn)];
		DrawRectRotaGraph(ComboUI::kLockOnStartX, ComboUI::kLockOnStartY, 0, 0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale, 0.0f, handle, true);
	}
	//ロックオン切換え
	DrawFormatString(ComboUI::kLockOnChangeStartX, ComboUI::kLockOnChangeStartY - ComboUI::kStringDistanceY, GetColor(0, 0, 0), "ロックオン切換え");
	{
		int handle = m_buttonHandles[static_cast<int>(ComboButton::RStickLR)];
		DrawRectRotaGraph(ComboUI::kLockOnChangeStartX, ComboUI::kLockOnChangeStartY, 0, 0, kButtonWidth, kButtonHeight, ComboUI::kButtonScale, 0.0f, handle, true);
	}


}
