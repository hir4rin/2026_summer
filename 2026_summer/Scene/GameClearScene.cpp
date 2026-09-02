#include "GameClearScene.h"
#include "../Input.h"
#include "GameScene.h"
#include "SceneController.h"
#include "../Game.h"
#include "../System.h"
#include "Player.h"
#include "../Camera/CameraManager.h"

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

	const Vector3 kPlayerStartPos = Vector3(0, 0, 300);//結果表示用プレイヤーの初期座標

	constexpr int kTitleY = 120;//"GAME CLEAR"の表示Y座標
	constexpr int kTitleXOffset = 100;//"GAME CLEAR"を画面中央からずらすX方向のオフセット

	constexpr int kReturnMessageY = 620;//「Aボタンで～」メッセージの表示Y座標
	constexpr int kReturnMessageXOffset = 140;//「Aボタンで～」メッセージを画面中央からずらすX方向のオフセット
}

GameClearScene::GameClearScene(SceneController& controller, const GameResult& result) :Scene(controller), m_result(result)
{
	m_updateFunc = static_cast<UpdateFunc_t>(&GameClearScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameClearScene::NormalDraw);

	//プレイヤーの初期化
	m_player = std::make_shared<Player>();
	m_player->Init();
	m_player->SetPos(kPlayerStartPos);

	//カメラの初期化(リザルト画面では定点でPlayerを映すResultCameraStateのみを使う)
	m_cameraManager = std::make_shared<CameraManager>();
	m_cameraManager->SetIsResult(true);
	m_cameraManager->Init(std::weak_ptr<Player>(m_player));
	m_cameraManager->ChangeStateFromScene(CameraManager::CameraStateName::ResultCamera);
	m_cameraManager->Update(m_player->GetPos());
	m_player->SetCameraManager(std::weak_ptr<CameraManager>(m_cameraManager));
	m_player->SetResultUp();

	//入力をrealじゃないと取らなくなる
	InputRecord record;
	record = { {1000000000,{}} };
	Input::GetInstance().StartRecord(record);

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
		if (input.IsTriggered("A"))
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
		if (input.IsTriggered("A"))
		{
			Input::GetInstance().StopRecord();
			//ゲームシーンに遷移する
			m_controller.ChangeScene(std::make_shared<GameScene>(m_controller));
			return;
		}
		break;
	}

	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();
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
	SetDrawScreen(DX_SCREEN_BACK); ClearDrawScreen();

	//定点カメラでPlayerを映す
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
		DrawFormatString(Game::GetScreenWidth() / 2 - static_cast<int>(Game::ScaleX(kReturnMessageXOffset)), static_cast<int>(Game::ScaleY(kReturnMessageY)), GetColor(255, 255, 255), "Aボタンでゲームシーンに戻る");
	}
}

void GameClearScene::FadeOutDraw()
{

}

float GameClearScene::GetItemTarget(ResultItem item) const
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

void GameClearScene::DrawItem(ResultItem item, int y) const
{
	float value = m_displayValues[static_cast<int>(item)];
	int x = static_cast<int>(Game::ScaleX(kItemStartX));
	y = static_cast<int>(Game::ScaleY(y));
	//漢字を含む文字列はGAGAGAGA-FREEフォントには無く、デフォルトフォントのまま表示する
	switch (item)
	{
	case ResultItem::ClearTime:
		DrawFormatString(x, y, GetColor(255, 255, 255), "クリアタイム : %.1f 秒", value);
		break;
	case ResultItem::Damage:
		DrawFormatString(x, y, GetColor(255, 255, 255), "与えたダメージ : %d", static_cast<int>(value));
		break;
	case ResultItem::Combo:
		DrawFormatString(x, y, GetColor(255, 255, 255), "コンボ数 : %d", static_cast<int>(value));
		break;
	case ResultItem::DamageTaken:
		DrawFormatString(x, y, GetColor(255, 255, 255), "トータルダメージ : %d", static_cast<int>(value));
		break;
	default:
		break;
	}
}

char GameClearScene::CalcRank() const
{
	float clearTime = m_result.clearTime;
	if (clearTime <= kRankTimeS)return 'S';
	if (clearTime <= kRankTimeA)return 'A';
	if (clearTime <= kRankTimeB)return 'B';
	return 'C';
}
