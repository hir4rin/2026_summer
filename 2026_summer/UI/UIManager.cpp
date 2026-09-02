#include "UIManager.h"
#include "UIBase.h"
#include "ComboHUD.h"

UIManager::UIManager()
{
	m_comboHUD = std::make_shared<ComboHUD>();
	m_uiLists.push_back(m_comboHUD);
}

UIManager::~UIManager()
{
}

void UIManager::Init()
{
}

void UIManager::Update()
{
	for (auto& ui : m_uiLists)
	{
		ui->Update();
	}
}
void UIManager::Draw()
{
	for (auto& ui : m_uiLists)
	{
		ui->Draw();
	}

}

void UIManager::ToggleComboUI()
{
	if (!m_comboHUD)return;
	m_comboHUD->ToggleVisible();
}
