#include "PlayerHUD.h"
#include "Player.h"
#include "../Game.h"
#include "../System.h"

namespace
{
	constexpr int kBarWidth = 1;
	constexpr int kBarHeight = 26;

	constexpr float kGaugeMaxValue = 100.0f;//HP/ゲージの最大値(パーセント計算用)
	constexpr int kDebugTextX = 10;//デバッグ表示のX座標
	constexpr int kDebugTextOffsetY = 20;//デバッグ表示のバーからの上方向オフセット

	constexpr int kPlayerHpSrcX = 800;
	constexpr int kPlayerHpSrcY = 200;

	constexpr int kUltGaugeBaseSrcSize = 640;//UltGaugeBase.pngの元の一辺の大きさ
	constexpr int kUltGaugeCircleSrcSize = 460;//UltGauge_circle.pngの元の一辺の大きさ
	constexpr float kUltGaugeDesignDiameter = 80.0f;//Ultゲージ(円形)の設計解像度での表示直径
	constexpr int kUltGaugeDimBright = 130;//Ultゲージが満タンでないときに少し薄暗く見せるための明るさ(0~255)
}

PlayerHUD::PlayerHUD()
{
	m_barHideHandle = LoadGraph("data/UI/gauge_brack.png");
	m_hpBarHandle = LoadGraph("data/UI/gauge_hp.png");
	m_playerHpHandle = LoadGraph("data/UI/PlayerHP.png");
	m_skillGaugeBarHandle = LoadGraph("data/UI/gauge_Skill.png");
	m_ultGaugeHandle = LoadGraph("data/UI/UltGaugeBase.png");
	m_ultGaugeCircleHandle = LoadGraph("data/UI/UltGauge_circle.png");
	m_ultGaugeBarHandle = LoadGraph("data/UI/gauge_Ult.png");
}

PlayerHUD::~PlayerHUD()
{

}

void PlayerHUD::Init()
{
}

void PlayerHUD::Update()
{
	auto player = m_player.lock();
	
}

void PlayerHUD::Draw() const
{
	auto player = m_player.lock();
	float hp = static_cast<float>(player->GetHp());
	float skillGauge = static_cast<float>(player->GetSkillGauge());
	float ultGauge = static_cast<float>(player->GetUltGauge());

	//画面比率でバーの座標・サイズを計算する
	int barHeight = static_cast<int>(Game::ScaleY(kBarHeight));
	int hpX = static_cast<int>(Game::ScaleX(UI::HpBarX));
	int hpY = static_cast<int>(Game::ScaleY(UI::HpBarY));
	int skillX = static_cast<int>(Game::ScaleX(UI::SkillGaugeBarX));
	int skillY = static_cast<int>(Game::ScaleY(UI::SkillGaugeBarY));
	int ultX = static_cast<int>(Game::ScaleX(UI::UltGaugeBarX));
	int ultY = static_cast<int>(Game::ScaleY(UI::UltGaugeBarY));

	//HPバーの描画
	//現在のHP表示位置に合わせて装飾画像を表示する(DrawRectRotaGraphは中心座標基準・等倍率なので、バー幅に合わせて拡大率を計算する)
	int hpBarWidth = static_cast<int>(Game::ScaleX(UI::kBarScale));
	double playerHpScale = static_cast<double>(hpBarWidth) / kPlayerHpSrcX * 1.5f;
	DrawRectRotaGraph(hpX + hpBarWidth * 2 / 3, hpY + barHeight / 2, 0, 0, kPlayerHpSrcX, kPlayerHpSrcY, playerHpScale, 0.0, m_playerHpHandle, TRUE);
	//DrawExtendGraph(hpX, hpY, static_cast<int>(Game::ScaleX(UI::HpBarX + UI::kBarScale)), hpY + barHeight, m_barHideHandle, false);//HPバーの背景
	//実際のhp
	//x2はx1(左端)を基準にHP比率分の幅を足す形にする(hp=0のときx1=x2になり、幅0で自然に消えるようにするため)
	float hpBarLeftX = hpX + hpBarWidth * 0.9f / 10;
	float hpBarFillWidth = Game::ScaleX(UI::kBarScale * (hp / kGaugeMaxValue)) * 1.138f;
	DrawExtendGraph(static_cast<int>(hpBarLeftX), hpY + barHeight * 2/10, static_cast<int>(hpBarLeftX + hpBarFillWidth), hpY + barHeight * 0.94f, m_hpBarHandle, false);//HPバー
	//スキルゲージバーの描画
	DrawExtendGraph(skillX, skillY, static_cast<int>(Game::ScaleX(UI::SkillGaugeBarX + UI::kBarScale)), skillY + barHeight, m_barHideHandle, false);//スキルゲージバーの背景
	DrawExtendGraph(skillX, skillY, static_cast<int>(Game::ScaleX(UI::SkillGaugeBarX + UI::kBarScale * (skillGauge / kGaugeMaxValue))), skillY + barHeight, m_skillGaugeBarHandle, false);//スキルゲージバー
	//必殺技ゲージの描画(円形ゲージ)//現在のUltゲージがあった位置ぐらいに表示する
	{
		int ultDiameter = static_cast<int>(Game::ScaleX(kUltGaugeDesignDiameter));
		int ultCenterX = ultX + ultDiameter / 2;
		int ultCenterY = ultY + ultDiameter / 2;
		double ultBaseScale = static_cast<double>(ultDiameter) / kUltGaugeBaseSrcSize * 1.5f;
		double ultCircleScale = static_cast<double>(ultDiameter) / kUltGaugeCircleSrcSize;

		//満タンでないときは少し薄暗く描画する
		bool isUltFull = ultGauge >= kGaugeMaxValue;
		if (!isUltFull)
		{
			SetDrawBright(kUltGaugeDimBright, kUltGaugeDimBright, kUltGaugeDimBright);
		}

		//土台画像
		DrawRotaGraph(ultCenterX, ultCenterY, ultBaseScale, 0.0, m_ultGaugeHandle, TRUE);
		//たまり具合に応じて円グラフ状に表示する(ultGaugeは0~100のパーセント値)
		DrawCircleGaugeF(static_cast<float>(ultCenterX), static_cast<float>(ultCenterY), ultGauge, m_ultGaugeCircleHandle, 0.0, ultCircleScale);

		if (!isUltFull)
		{
			SetDrawBright(255, 255, 255);//必ずリセット！
		}
	}


	//デバッグ用に数値を表示//英数字のみの文字列なのでYDWgagagagaフォントを使う
	int ydwFontHandle = System::GetInstance().GetYdwGagagagaFontHandle();
	//DrawFormatStringToHandle(static_cast<int>(Game::ScaleX(kDebugTextX)), hpY - static_cast<int>(Game::ScaleY(kDebugTextOffsetY)), GetColor(0, 0, 0), ydwFontHandle, "Player HP: %d", player->GetHp());
	//DrawFormatStringToHandle(static_cast<int>(Game::ScaleX(kDebugTextX)), skillY - static_cast<int>(Game::ScaleY(kDebugTextOffsetY)), GetColor(0, 0, 0), ydwFontHandle, "Player SkillGauge: %d", player->GetSkillGauge());
	//DrawFormatStringToHandle(static_cast<int>(Game::ScaleX(kDebugTextX)), ultY - static_cast<int>(Game::ScaleY(kDebugTextOffsetY)), GetColor(0, 0, 0), ydwFontHandle, "Player UltGauge: %d", player->GetUltGauge());

}
