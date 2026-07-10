#include "GameClearScene.h"
#include "../Input.h"
#include "GameScene.h"
#include "SceneController.h"

GameClearScene::GameClearScene(SceneController& controller) :Scene(controller)
{
	m_updateFunc = static_cast<UpdateFunc_t>(&GameClearScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameClearScene::NormalDraw);
}

GameClearScene::~GameClearScene()
{
}

void GameClearScene::Update()
{
	(this->*m_updateFunc)();
}

void GameClearScene::FadeInUpdate()
{
}

void GameClearScene::NormalUpdate()
{
	auto& input = Input::GetInstance();


	if (input.IsTriggered("A"))
	{
		//ゲームシーンに遷移する
		m_controller.ChangeScene(std::make_shared<GameScene>(m_controller));
	}
}

void GameClearScene::FadeOutUpdate()
{
}

void GameClearScene::Draw()
{
	(this->*m_drawFunc)();
}

void GameClearScene::FadeInDraw()
{
}

void GameClearScene::NormalDraw()
{
	DrawFormatString(300, 300, GetColor(255, 255, 255), "GameClearScene");
}

void GameClearScene::FadeOutDraw()
{
}
