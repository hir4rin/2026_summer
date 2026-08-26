#include "GameOverScene.h"
#include "SceneController.h"
#include "TitleScene.h"
#include "../Input.h"

GameOverScene::GameOverScene(SceneController& controller) :Scene(controller)
{
	m_updateFunc = static_cast<UpdateFunc_t>(&GameOverScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameOverScene::NormalDraw);
}

GameOverScene::~GameOverScene()
{
}

void GameOverScene::Update()
{
	(this->*m_updateFunc)();
}

void GameOverScene::FadeInUpdate()
{
}

void GameOverScene::NormalUpdate()
{
	auto& input = Input::GetInstance();


	if (input.IsTriggered("A"))
	{
		//タイトルシーンに遷移する
		m_controller.ChangeScene(std::make_shared<TitleScene>(m_controller));
	}
	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();
}

void GameOverScene::FadeOutUpdate()
{
}

void GameOverScene::Draw()
{
	(this->*m_drawFunc)();
}

void GameOverScene::FadeInDraw()
{
}

void GameOverScene::NormalDraw()
{
	DrawFormatString(300, 300, GetColor(255, 255, 255), "GameOverScene");
}

void GameOverScene::FadeOutDraw()
{
}
