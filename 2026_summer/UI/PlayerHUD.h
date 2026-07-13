#pragma once
#include "UIBase.h"
#include <memory>

class Player;

class PlayerHUD :
    public UIBase
{
public:
    PlayerHUD();
    ~PlayerHUD();

    void Init() override;
	void SetPlayer(std::weak_ptr<Player> player) { m_player = player; }

    void Update() override;
	void Draw() const override;
private:
    std::weak_ptr<Player> m_player;
	int m_barHideHandle = -1;//バーの背景
	int m_hpBarHandle = -1;//HPバー
	int m_skillGaugeBarHandle = -1;//スキルゲージバー
	int m_ultGaugeBarHandle = -1;//アルティメットゲージバー

};

