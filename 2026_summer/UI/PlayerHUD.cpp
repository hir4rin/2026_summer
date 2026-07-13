#include "PlayerHUD.h"
#include "Player.h"
#include "../Game.h"

namespace
{
	constexpr int kBarWidth = 1;
	constexpr int kBarHeight = 26;
}

PlayerHUD::PlayerHUD()
{
	m_barHideHandle = LoadGraph("data/UI/gauge_brack.png");
	m_hpBarHandle = LoadGraph("data/UI/gauge_hp.png");
	m_skillGaugeBarHandle = LoadGraph("data/UI/gauge_Skill.png");
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
	float hp = player->GetHp();
	float skillGauge = player->GetSkillGauge();
	float ultGauge = player->GetUltGauge();



	//HPバーの描画
	DrawExtendGraph(UI::HpBarX, UI::HpBarY, UI::HpBarX + UI::kBarScale,			     UI::HpBarY + kBarHeight, m_barHideHandle, false);//HPバーの背景　
	DrawExtendGraph(UI::HpBarX, UI::HpBarY, UI::HpBarX + UI::kBarScale * (hp / 100), UI::HpBarY +kBarHeight, m_hpBarHandle, false);//HPバー
	//スキルゲージバーの描画
	DrawExtendGraph(UI::SkillGaugeBarX, UI::SkillGaugeBarY, UI::SkillGaugeBarX + UI::kBarScale,						 UI::SkillGaugeBarY + kBarHeight, m_barHideHandle, false);//スキルゲージバーの背景
	DrawExtendGraph(UI::SkillGaugeBarX, UI::SkillGaugeBarY,	UI::SkillGaugeBarX + UI::kBarScale * (skillGauge / 100), UI::SkillGaugeBarY + kBarHeight, m_skillGaugeBarHandle, false);//スキルゲージバー
	//必殺技ゲージバーの描画
	DrawExtendGraph(UI::UltGaugeBarX, UI::UltGaugeBarY, UI::UltGaugeBarX + UI::kBarScale,					 UI::UltGaugeBarY + kBarHeight, m_barHideHandle, false);//必殺技ゲージバーの背景
	DrawExtendGraph(UI::UltGaugeBarX, UI::UltGaugeBarY, UI::UltGaugeBarX + UI::kBarScale * (ultGauge / 100), UI::UltGaugeBarY + kBarHeight, m_ultGaugeBarHandle, false);//必殺技ゲージバー

	//デバッグ用に数値を表示
	DrawFormatString(10, UI::HpBarY-20, GetColor(0, 0, 0), "Player HP: %d", player->GetHp());
	DrawFormatString(10, UI::SkillGaugeBarY - 20, GetColor(0, 0, 0), "Player SkillGauge: %d", player->GetSkillGauge());
	DrawFormatString(10, UI::UltGaugeBarY - 20, GetColor(0, 0, 0), "Player UltGauge: %d", player->GetUltGauge());

}
