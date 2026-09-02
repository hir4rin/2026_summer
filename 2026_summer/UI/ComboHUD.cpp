#include "ComboHUD.h"
#include "../Game.h"
#include "../System.h"


namespace
{
	constexpr int kButtonWidth = 64;
	constexpr int kButtonHeight = 64;

	constexpr int kHoldSrcWidth = 450;//hold.pngの元の横幅
	constexpr int kHoldSrcHeight = 400;//hold.pngの元の縦幅
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
	m_buttonHandles[static_cast<int>(ComboButton::Hold)] = LoadGraph("data/UI/hold.png");

}

void ComboHUD::Update()
{
}

int ComboHUD::SX(int designX) const
{
	return static_cast<int>(Game::ScaleX(static_cast<float>(designX)));
}

int ComboHUD::SY(int designY) const
{
	return static_cast<int>(Game::ScaleY(static_cast<float>(designY)));
}

void ComboHUD::DrawComboButton(int x, int y, int buttonIndex, int handle, double scale) const
{
	if (buttonIndex == static_cast<int>(ComboButton::Hold))
	{
		double normalize = static_cast<double>(kButtonWidth) / kHoldSrcWidth;
		DrawRectRotaGraph(x, y, 0, 0, kHoldSrcWidth, kHoldSrcHeight, scale * normalize, 0.0f, handle, true);
		return;
	}
	DrawRectRotaGraph(x, y, 0, 0, kButtonWidth, kButtonHeight, scale, 0.0f, handle, true);
}

void ComboHUD::Draw() const
{
	//ここで、コンボのボタンを描画する処理を実装する
	//ボタン画像はGame::GetScale()で拡大率を掛けて、位置と一緒に画面比率で表示する
	double buttonScale = ComboUI::kButtonScale * Game::GetScale();

	//コンボUIの文字は漢字も含めて表示できるコーポレート明朝フォントを使う
	int comboFontHandle = System::GetInstance().GetCorporateMinchoFontHandle();

	//弱攻撃のコンボボタン
	DrawFormatStringToHandle(SX(ComboUI::kCombo1StartX), SY(ComboUI::kCombo1StartY - ComboUI::kStringDistanceY), GetColor(0, 0, 0), comboFontHandle, "弱攻撃コンボ");
	for(int i = 0; i < std::size(ComboUI::kCombo1); ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kCombo1[i])];
		DrawRectRotaGraph(SX(ComboUI::kCombo1StartX + i * ComboUI::ButtonToButtonDistanceX), SY(ComboUI::kCombo1StartY),0,0, kButtonWidth, kButtonHeight, buttonScale,0.0f, handle, true);
	}
	//強攻撃のコンボボタン
	DrawFormatStringToHandle(SX(ComboUI::kCombo2StartX), SY(ComboUI::kCombo2StartY - ComboUI::kStringDistanceY), GetColor(0, 0, 0), comboFontHandle, "強攻撃コンボ");
	for(int i = 0; i < std::size(ComboUI::kCombo2); ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kCombo2[i])];
		DrawRectRotaGraph(SX(ComboUI::kCombo2StartX + i * ComboUI::ButtonToButtonDistanceX), SY(ComboUI::kCombo2StartY),0,0, kButtonWidth, kButtonHeight, buttonScale,0.0f, handle, true);
	}
	//打ち上げコンボボタン
	DrawFormatStringToHandle(SX(ComboUI::kAirComboStartX), SY(ComboUI::kAirComboStartY - ComboUI::kStringDistanceY), GetColor(0, 0, 0), comboFontHandle, "打ち上げコンボ");
	for(int i = 0; i < std::size(ComboUI::kAirCombo); ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kAirCombo[i])];
		DrawRectRotaGraph(SX(ComboUI::kAirComboStartX + i * ComboUI::ButtonToButtonDistanceX), SY(ComboUI::kAirComboStartY),0,0, kButtonWidth, kButtonHeight, buttonScale,0.0f, handle, true);
	}

	//スキル攻撃のコンボボタン
	DrawFormatStringToHandle(SX(ComboUI::kSkillStartX), SY(ComboUI::kSkillStartY - ComboUI::kStringDistanceY), GetColor(0, 0, 0), comboFontHandle, "スキル攻撃コンボ");
	for(int i = 0; i < std::size(ComboUI::kSkillCombo); ++i)
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kSkillCombo[i])];
		DrawComboButton(SX(ComboUI::kSkillStartX + i * ComboUI::ButtonToButtonDistanceX), SY(ComboUI::kSkillStartY), static_cast<int>(ComboUI::kSkillCombo[i]), handle, buttonScale);
	}
	//必殺技のボタン
	DrawFormatStringToHandle(SX(ComboUI::kUltStartX), SY(ComboUI::kUltStartY - ComboUI::kStringDistanceY), GetColor(0, 0, 0), comboFontHandle, "必殺技コンボ");
	{
		for(int i = 0; i < std::size(ComboUI::kUlt); ++i)
		{
			int handle = m_buttonHandles[static_cast<int>(ComboUI::kUlt[i])];
			DrawComboButton(SX(ComboUI::kUltStartX + i * ComboUI::ButtonToButtonDistanceX), SY(ComboUI::kUltStartY), static_cast<int>(ComboUI::kUlt[i]), handle, buttonScale);
		}
	}
	//ロックオン
	DrawFormatStringToHandle(SX(ComboUI::kLockOnStartX), SY(ComboUI::kLockOnStartY - ComboUI::kStringDistanceY), GetColor(0, 0, 0), comboFontHandle, "ロックオン");
	{
		int handle = m_buttonHandles[static_cast<int>(ComboUI::kLockOn)];
		DrawRectRotaGraph(SX(ComboUI::kLockOnStartX), SY(ComboUI::kLockOnStartY), 0, 0, kButtonWidth, kButtonHeight, buttonScale, 0.0f, handle, true);
	}
	//ロックオン切換え
	DrawFormatStringToHandle(SX(ComboUI::kLockOnChangeStartX), SY(ComboUI::kLockOnChangeStartY - ComboUI::kStringDistanceY), GetColor(0, 0, 0), comboFontHandle, "ロックオン切換え");
	{
		int handle = m_buttonHandles[static_cast<int>(ComboButton::RStickLR)];
		DrawRectRotaGraph(SX(ComboUI::kLockOnChangeStartX), SY(ComboUI::kLockOnChangeStartY), 0, 0, kButtonWidth, kButtonHeight, buttonScale, 0.0f, handle, true);
	}


}
