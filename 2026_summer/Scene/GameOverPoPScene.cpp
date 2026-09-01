#include "GameOverPoPScene.h"
#include "../Input.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "SceneController.h"
#include "../Game.h"
#include "../System.h"

namespace
{
	constexpr int kFadeFrame = 20;//フェードイン・フェードアウトにかけるフレーム数
}

GameOverPoPScene::GameOverPoPScene(SceneController& controller, GameScene& gameScene) :Scene(controller), m_gameScene(gameScene)
{
	//黒画面からフェードインしてスタートする
	m_updateFunc = static_cast<UpdateFunc_t>(&GameOverPoPScene::FadeInUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameOverPoPScene::FadeInDraw);

	//BGMを流す
	System::GetInstance().GetSoundManager().PlayBgm("GameOverBGM");
	System::GetInstance().SetTimeScale(1.0f);//念のため、時間の流れを通常に戻す

	m_deathTextHandle = LoadGraph("data/UI/deathText.png");
}

GameOverPoPScene::~GameOverPoPScene()
{
	DeleteGraph(m_deathTextHandle);
}

void GameOverPoPScene::Update()
{
	//下に積まれたGameSceneも更新する(背景で敵やエフェクトが動き続けるようにする)
	m_gameScene.Update();

	(this->*m_updateFunc)();
}

void GameOverPoPScene::FadeInUpdate()
{
	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	m_fadeCount++;
	if (m_fadeCount >= kFadeFrame)
	{
		//フェードインが終わったので、通常動作に戻す
		m_fadeCount = 0;
		m_updateFunc = static_cast<UpdateFunc_t>(&GameOverPoPScene::NormalUpdate);
		m_drawFunc = static_cast<DrawFunc_t>(&GameOverPoPScene::NormalDraw);
	}
}

void GameOverPoPScene::NormalUpdate()
{
	auto& input = Input::GetInstance();

	if (input.IsTriggered("A"))
	{
		//フェードアウトしてからタイトルシーンに初期化する
		m_fadeCount = 0;
		m_updateFunc = static_cast<UpdateFunc_t>(&GameOverPoPScene::FadeOutUpdate);
		m_drawFunc = static_cast<DrawFunc_t>(&GameOverPoPScene::FadeOutDraw);
		return;
	}

	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();
}

void GameOverPoPScene::FadeOutUpdate()
{
	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	m_fadeCount++;
	if (m_fadeCount >= kFadeFrame)
	{
		//一応
		System::GetInstance().SetIsEventPlaying(false);
		System::GetInstance().SetIsLastHitEventPlaying(false);
		System::GetInstance().SetUltEnd();

		//画面が真っ暗になったタイミングで、タイトルシーンに初期化する(今積まれているシーンを全部破棄してから、新しいTitleSceneを1つだけ作る)
		m_controller.ResetScene<TitleScene>();
		return;
	}
}

void GameOverPoPScene::Draw()
{
	(this->*m_drawFunc)();
}

void GameOverPoPScene::FadeInDraw()
{
	NormalDraw();

	//だんだん透明にしていく
	int alpha = 255 * (kFadeFrame - m_fadeCount) / kFadeFrame;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameOverPoPScene::NormalDraw()
{
	//画面を少し黒くする(PauseSceneと同じやり方)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 164);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← 必ずリセット！

	//下に積まれたGameSceneはSceneController::Drawがscenes_を順番に描画することで自動的に映る//ここではゲームオーバーのUIだけ描く
	//死亡テキストを画面中央に表示する
	DrawRotaGraph(Game::kScreenWidth / 2, Game::kScreenHeight / 2, 0.3f, 0.0, m_deathTextHandle, TRUE);

	DrawFormatString(Game::kScreenWidth / 2 - 140, 620, GetColor(255, 255, 255), "Aボタンでタイトルに戻る");
}

void GameOverPoPScene::FadeOutDraw()
{
	NormalDraw();

	//だんだん暗くしていく
	int alpha = 255 * m_fadeCount / kFadeFrame;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
