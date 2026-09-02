#include "StageClearPoPScene.h"
#include "../Input.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "SceneController.h"
#include "../Game.h"
#include "../System.h"
#include "Player.h"
#include "../Camera/CameraManager.h"
#include "../Camera/CameraState/CameraStateBase.h"
#ifdef _DEBUG
#include "../imguiApp.h"
#endif

namespace
{
	constexpr int kCountUpFrames = 40;//1項目をカウントアップする時間
	constexpr int kItemPauseFrames = 20;//カウントアップが終わってから次の項目に移るまでの間

	constexpr int kRankPopFrames = 30;//ランクが表示されてから入力待ちになるまでの間

	constexpr int kItemStartX = 440;
	constexpr int kItemStartY = 260;
	constexpr int kItemDistanceY = 50;

	constexpr int kRankX = 640;
	constexpr int kRankY = 500;

	//クリアタイムでランクを決める(秒)//短いほど高ランク//適当
	constexpr float kRankTimeS = 120.0f;
	constexpr float kRankTimeA = 180.0f;
	constexpr float kRankTimeB = 240.0f;
	//これを超えたらランクC

	constexpr int kFadeFrame = 20;//フェードイン・フェードアウトにかけるフレーム数

	const Vector3 kPlayerStartPos = Vector3(0, 0, 300);//結果表示用プレイヤーの初期座標

	constexpr int kDarkenAlpha = 164;//画面を少し黒くするアルファ値

	constexpr int kTitleY = 120;//"GAME CLEAR"の表示Y座標
	constexpr int kTitleXOffset = 100;//"GAME CLEAR"を画面中央からずらすX方向のオフセット

	constexpr int kReturnMessageY = 620;//"Press A Button"メッセージの表示Y座標
}

StageClearPoPScene::StageClearPoPScene(SceneController& controller, const GameResult& result) :Scene(controller), m_result(result)
{
	//黒画面からフェードインしてスタートする
	m_updateFunc = static_cast<UpdateFunc_t>(&StageClearPoPScene::FadeInUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&StageClearPoPScene::FadeInDraw);

	//BGMを流す
	System::GetInstance().GetSoundManager().PlayBgm("ResultBGM");
	System::GetInstance().SetTimeScale(1.0f);//念のため、時間の流れを通常に戻す

	//GameClearSceneと同じく、結果表示専用のプレイヤーとカメラを自前で用意する
	m_player = std::make_shared<Player>();
	m_player->Init();
	m_player->SetPos(kPlayerStartPos);

	//カメラの初期化(定点でPlayerを映すResultCameraStateのみを使う)
	m_cameraManager = std::make_shared<CameraManager>();
	m_cameraManager->SetIsResult(true);
	m_cameraManager->Init(std::weak_ptr<Player>(m_player));
	m_cameraManager->ChangeStateFromScene(CameraManager::CameraStateName::ResultCamera);
	m_cameraManager->Update(m_player->GetPos());
	m_player->SetCameraManager(std::weak_ptr<CameraManager>(m_cameraManager));
	m_player->SetResultUp();
}

StageClearPoPScene::~StageClearPoPScene()
{
}

void StageClearPoPScene::Update()
{
	(this->*m_updateFunc)();
}

void StageClearPoPScene::FadeInUpdate()
{
	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	m_fadeCount++;
	if (m_fadeCount >= kFadeFrame)
	{
		//フェードインが終わったので、通常動作に戻す
		m_fadeCount = 0;
		m_updateFunc = static_cast<UpdateFunc_t>(&StageClearPoPScene::NormalUpdate);
		m_drawFunc = static_cast<DrawFunc_t>(&StageClearPoPScene::NormalDraw);
	}
}

void StageClearPoPScene::NormalUpdate()
{
	auto& input = Input::GetInstance();

	//結果表示用プレイヤー・カメラの更新(GameClearSceneと同じ)
	m_cameraManager->Update(m_player->GetPos());
	m_player->Update(*m_cameraManager->GetHighestPriorityCamera());

	if (input.IsRealTriggered("X"))
	{
		m_player->ChangeResultMove();
	}

	switch (m_phase)
	{
	case Phase::Counting:
	{
		ResultItem item = static_cast<ResultItem>(m_itemIndex);
		float target = GetItemTarget(item);

		float t = static_cast<float>(m_itemFrame) / kCountUpFrames;
		if (t > 1.0f)t = 1.0f;
		m_displayValues[m_itemIndex] = target * t;

		m_itemFrame++;
		if (m_itemFrame >= kCountUpFrames + kItemPauseFrames)
		{
			m_displayValues[m_itemIndex] = target;//誤差をなくす
			m_itemIndex++;
			m_itemFrame = 0;
			if (m_itemIndex >= static_cast<int>(ResultItem::Size))
			{
				m_rank = CalcRank();
				m_phase = Phase::Rank;
			}
		}
		//ボタン入力で一気に表示する
		if (input.IsRealTriggered("A"))
		{
			for (int i = 0; i < static_cast<int>(ResultItem::Size); i++)
			{
				m_displayValues[i] = GetItemTarget(static_cast<ResultItem>(i));
			}
			m_rank = CalcRank();
			m_phase = Phase::Rank;
		}
		break;
	}
	case Phase::Rank:
		m_rankFrame++;
		if (m_rankFrame >= kRankPopFrames)
		{
			m_phase = Phase::Done;
		}
		if (input.IsTriggered("A"))
		{
			m_phase = Phase::Done;
		}
		break;
	case Phase::Done:
		if (input.IsRealTriggered("A"))
		{
			//フェードアウトしてからゲームシーンに初期化する
			m_fadeCount = 0;
			m_updateFunc = static_cast<UpdateFunc_t>(&StageClearPoPScene::FadeOutUpdate);
			m_drawFunc = static_cast<DrawFunc_t>(&StageClearPoPScene::FadeOutDraw);
			return;
		}
		break;
	}

	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();
}

void StageClearPoPScene::FadeOutUpdate()
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

		//画面が真っ暗になったタイミングで、ゲームシーンに初期化する(今積まれているシーンを全部破棄してから、新しいGameSceneを1つだけ作る)
		m_controller.ResetScene<TitleScene>();
		return;
	}
}

void StageClearPoPScene::Draw()
{
	(this->*m_drawFunc)();
}

void StageClearPoPScene::FadeInDraw()
{
	NormalDraw();

	//だんだん透明にしていく
	int alpha = 255 * (kFadeFrame - m_fadeCount) / kFadeFrame;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void StageClearPoPScene::NormalDraw()
{
	//画面を少し黒くする(PauseSceneと同じやり方)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, kDarkenAlpha);
	DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← 必ずリセット！

	//下に積まれたGameSceneはSceneController::Drawがscenes_を順番に描画することで自動的に映る
	//ここで、結果表示用のカメラ・プレイヤー(GameClearSceneと同じ、赤くしたもの)を前面に描画する
	m_cameraManager->ApplyCameraSettings();
	m_player->Draw();

	//"GAME CLEAR"は英字のみの文字列なのでYDWgagagagaフォントを使う
	int ydwFontHandle = System::GetInstance().GetYdwGagagagaFontHandle();
	DrawFormatStringToHandle(Game::GetScreenWidth() / 2 - static_cast<int>(Game::ScaleX(kTitleXOffset)), static_cast<int>(Game::ScaleY(kTitleY)), GetColor(255, 255, 255), ydwFontHandle, "GAME CLEAR");

	//カウントアップ中の項目までを表示する(まだ来ていない項目は表示しない)
	int visibleCount = (m_phase == Phase::Counting) ? m_itemIndex + 1 : static_cast<int>(ResultItem::Size);
	for (int i = 0; i < visibleCount; i++)
	{
		DrawItem(static_cast<ResultItem>(i), kItemStartY + i * kItemDistanceY);
	}

	if (m_phase == Phase::Rank || m_phase == Phase::Done)
	{
		//"RANK"は英字のみの文字列なのでYDWgagagagaフォントを使う
		DrawFormatStringToHandle(static_cast<int>(Game::ScaleX(kRankX)), static_cast<int>(Game::ScaleY(kRankY)), GetColor(255, 220, 80), ydwFontHandle, "RANK %c", m_rank);
	}

	if (m_phase == Phase::Done)
	{
		//"Press A Button"は英字のみの文字列なのでYDWgagagagaフォントを使う
		const char* pressButtonText = "Press A Button";
		int textWidth = GetDrawStringWidthToHandle(pressButtonText, -1, ydwFontHandle);
		DrawStringToHandle(Game::GetScreenWidth() / 2 - textWidth / 2, static_cast<int>(Game::ScaleY(kReturnMessageY)), pressButtonText, GetColor(255, 255, 255), ydwFontHandle);
	}

#ifdef _DEBUG
	//PhotoModeと同じImGuiのカメラデバッグウィンドウを表示する(Override Cameraを有効にするとカメラをいじれる)
	if (m_cameraManager)
	{
		imguiApp::GetInstance().DrawCameraDebugWindow(
			m_cameraManager->GetHighestPriorityCamera()->GetCameraPos(),
			m_cameraManager->GetHighestPriorityCamera()->GetCameraTarget());
	}
#endif
}

void StageClearPoPScene::FadeOutDraw()
{
	NormalDraw();

	//だんだん暗くしていく
	int alpha = 255 * m_fadeCount / kFadeFrame;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

float StageClearPoPScene::GetItemTarget(ResultItem item) const
{
	switch (item)
	{
	case ResultItem::ClearTime:
		return m_result.clearTime;
	case ResultItem::Damage:
		return static_cast<float>(m_result.totalDamage);
	case ResultItem::Combo:
		return static_cast<float>(m_result.comboCount);
	case ResultItem::DamageTaken:
		return static_cast<float>(m_result.damageTakenCount);
	default:
		return 0.0f;
	}
}

void StageClearPoPScene::DrawItem(ResultItem item, int y) const
{
	float value = m_displayValues[static_cast<int>(item)];
	int x = static_cast<int>(Game::ScaleX(kItemStartX));
	y = static_cast<int>(Game::ScaleY(y));
	//漢字を含まない文字列に揃えたので、YDWgagagagaフォントを使う
	int ydwFontHandle = System::GetInstance().GetYdwGagagagaFontHandle();
	switch (item)
	{
	case ResultItem::ClearTime:
		DrawFormatStringToHandle(x, y, GetColor(255, 255, 255), ydwFontHandle, "クリアタイム : %.1f", value);
		break;
	case ResultItem::Damage:
		DrawFormatStringToHandle(x, y, GetColor(255, 255, 255), ydwFontHandle, "ダメージ : %d", static_cast<int>(value));
		break;
	case ResultItem::Combo:
		DrawFormatStringToHandle(x, y, GetColor(255, 255, 255), ydwFontHandle, "コンボスウ : %d", static_cast<int>(value));
		break;
	case ResultItem::DamageTaken:
		DrawFormatStringToHandle(x, y, GetColor(255, 255, 255), ydwFontHandle, "ヒットダメージ : %d", static_cast<int>(value));
		break;
	default:
		break;
	}
}

char StageClearPoPScene::CalcRank() const
{
	float clearTime = m_result.clearTime;
	if (clearTime <= kRankTimeS)return 'S';
	if (clearTime <= kRankTimeA)return 'A';
	if (clearTime <= kRankTimeB)return 'B';
	return 'C';
}
