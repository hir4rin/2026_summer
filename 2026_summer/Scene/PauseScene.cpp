#include "PauseScene.h"
#include "SceneController.h"
#include "../Input.h"
#include "../Game.h"
#include "../System.h"
#include "../Application.h"

namespace
{
	constexpr int kDarkenAlpha = 164;//画面を少し黒くするアルファ値

	constexpr int kDebugTextX = 300;//仮のデバッグ表示のX座標
	constexpr int kDebugTextY = 300;//仮のデバッグ表示のY座標

	constexpr int kScreenModeTextX = 300;//画面モード表示のX座標
	constexpr int kScreenModeTextY = 340;//画面モード表示のY座標
}

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

	if (input.IsTriggered("B"))
	{
		//ウィンドウ/フルスクリーンを切り替える
		auto& app = Application::GetInstance();
		app.SetFullScreen(!app.IsFullScreen());
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
	//画面を少し黒くする(実解像度いっぱいに塗る)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, kDarkenAlpha);
	DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← 必ずリセット！


	//下に積まれたGameSceneはSceneController::Drawがscenes_を順番に描画することで自動的に映る//ここではポーズ中のUIだけ描く
	DrawFormatString(static_cast<int>(Game::ScaleX(kDebugTextX)), static_cast<int>(Game::ScaleY(kDebugTextY)), GetColor(255, 255, 255), "PauseScene");

	//Bボタンでウィンドウ/フルスクリーンを切り替えられることを表示する
	bool isFullScreen = Application::GetInstance().IsFullScreen();
	DrawFormatString(static_cast<int>(Game::ScaleX(kScreenModeTextX)), static_cast<int>(Game::ScaleY(kScreenModeTextY)), GetColor(255, 255, 255),
		"Bボタンで画面モード切替 : %s", isFullScreen ? "フルスクリーン" : "ウィンドウ");
}

void PauseScene::FadeOutDraw()
{
}
