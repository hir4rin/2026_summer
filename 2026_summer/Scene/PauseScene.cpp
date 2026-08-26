#include "PauseScene.h"
#include "SceneController.h"
#include "../Input.h"
#include "../Game.h"
#include "../System.h"

PauseScene::PauseScene(SceneController& controller) :Scene(controller)
{
	m_updateFunc = static_cast<UpdateFunc_t>(&PauseScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&PauseScene::NormalDraw);
}

PauseScene::~PauseScene()
{
}

void PauseScene::Update()
{
	(this->*m_updateFunc)();
}

void PauseScene::FadeInUpdate()
{
}

void PauseScene::NormalUpdate()
{
	auto& input = Input::GetInstance();

	if (input.IsTriggered("Y"))
	{
		//フォトモードを用意
		System::GetInstance().SetPhotoMode(true);
		//ポーズを解除して、GameSceneに戻る
		m_controller.PopScene();
		return;
	}


	if (input.IsTriggered("Start"))
	{

		System::GetInstance().SetPhotoMode(false);

		//ポーズを解除して、GameSceneに戻る
		m_controller.PopScene();
		return;
	}
}

void PauseScene::FadeOutUpdate()
{
}

void PauseScene::Draw()
{
	(this->*m_drawFunc)();
}

void PauseScene::FadeInDraw()
{
}

void PauseScene::NormalDraw()
{
	//画面を少し黒くする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 164);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← 必ずリセット！DrawGraph(Game::kScreenWidth, Game::kScreenHeight, true);


	//下に積まれたGameSceneはSceneController::Drawがscenes_を順番に描画することで自動的に映る//ここではポーズ中のUIだけ描く
	DrawFormatString(300, 300, GetColor(255, 255, 255), "PauseScene");
}

void PauseScene::FadeOutDraw()
{
}
