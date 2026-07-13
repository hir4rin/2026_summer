#include "UIManager.h"
#include "UIBase.h"

UIManager::UIManager()
{
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
