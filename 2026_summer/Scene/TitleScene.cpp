#include "TitleScene.h"
#include "../Game.h"
#include "../Input.h"
#include"SceneController.h"
#include "GameScene.h"


namespace
{
	constexpr int kTitleLogoX = 707;
	constexpr int kTitleLogoY = 275;
}



TitleScene::TitleScene(SceneController& controller) :Scene(controller)
{
	m_updateFunc = static_cast<UpdateFunc_t>(&TitleScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&TitleScene::NormalDraw);

	//dataの読み込み
	m_titleLogoHandle = LoadGraph("data/UI/TitleLogo.png");

}

TitleScene::~TitleScene()
{
	//dataの解放
	DeleteGraph(m_titleLogoHandle);
}

void TitleScene::Update()
{
	(this->*m_updateFunc)();
}

void TitleScene::FadeInUpdate()
{
}

void TitleScene::NormalUpdate()
{
	auto& input = Input::GetInstance();


	if(input.IsTriggered("A"))
	{
		//ゲームシーンに遷移する
		m_controller.ChangeScene(std::make_shared<GameScene>(m_controller));
	}
}

void TitleScene::FadeOutUpdate()
{
}

void TitleScene::Draw()
{
	(this->*m_drawFunc)();
}

void TitleScene::FadeInDraw()
{
}

void TitleScene::NormalDraw()
{
	DrawRectRotaGraph(Game::kScreenWidth /2, Game::kScreenHeight / 2,0,0,kTitleLogoX,kTitleLogoY,
		1.0, 0.0, m_titleLogoHandle, TRUE);
}

void TitleScene::FadeOutDraw()
{
}
