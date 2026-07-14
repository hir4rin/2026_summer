#include "UIManager.h"
#include "UIBase.h"
#include "ComboHUD.h"

UIManager::UIManager()
{
	auto comboHUD = std::make_shared<ComboHUD>();
	m_uiLists.push_back(comboHUD);
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
