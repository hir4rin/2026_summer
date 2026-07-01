#include "GameScene.h"
#include "../Camera/CameraManager.h"
#include "../Camera/PlayerCamera.h"
#include "Player.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"
#include "../Stage/Stage.h"
#include "../Game.h"
#include "../Input.h"
#include "EffekseerForDXLib.h"
#include "../Managers/EnemyManager.h"
#include "../SubWindow/SubWindow.h"

namespace
{
	constexpr int kGridRange = 2400;//グリッドのサイズ
}

GameScene::GameScene(SceneController& controller) :Scene(controller)
{
	//基底クラスに継承先のポインタをキャストして代入する
	m_updateFunc = static_cast<UpdateFunc_t>(&GameScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameScene::NormalDraw);

	//プレイヤーの初期化
	m_player = std::make_shared<Player>();
	m_player->Init();
	//敵の初期化
	/*m_enemySwordman = std::make_shared<EnemySwordman>(std::weak_ptr<Player>(m_player));
	m_enemySwordman->Init();*/
	m_enemyManager = std::make_shared<EnemyManager>(std::weak_ptr<Player>(m_player));


	m_cameraManager = std::make_unique<CameraManager>();
	//カメラの初期化
	m_cameraManager->Update(m_player->GetPos());

	m_stage = std::make_shared<Stage>();
	CollisionManager::GetInstance().Init();
	//レンダーターゲットの作成
	m_RT1 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	m_RT2 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	m_RT3 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);

}
GameScene::~GameScene()
{

}

void GameScene::Update()
{
	(this->*m_updateFunc)();
}

void GameScene::FadeInUpdate()
{
}

void GameScene::NormalUpdate()
{
	//ロックオンするか
	RockOnCamera();

	m_cameraManager->Update(m_player->GetPos() - Vector3(0, m_player->GetCenter(), 0));
	m_player->Update(*m_cameraManager->GetHighestPriorityCamera());
	m_enemyManager->Update();
	m_stage->Update();
	CollisionManager::GetInstance().Update();
}

void GameScene::FadeOutUpdate()
{
}

void GameScene::Draw()
{
	(this->*m_drawFunc)();
}

void GameScene::FadeInDraw()
{
}

void GameScene::NormalDraw()
{
	//UltDrawを作ったほうがいいかも
	//レンダリングを4つに分ける(map,Effect,UI,Character)
	//map,effectにシェーダーをかける

	//RT1
	SetDrawScreen(m_RT1); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	//赤くする
	m_stage->Draw();
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(255, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← 必ずリセット！
	//SetDrawBright(255, 255, 255);
	// Effekseerにより再生中のエフェクトを描画する。

	//RT2
	SetDrawScreen(m_RT2); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	//シェーダーで色をとって白くする
	//SetDrawBright(0, 0, 0);  // R=255, G=255, B=255	
	m_player->EffectDraw();
	//描画前に色を設定
	DrawEffekseer3D();
	DrawFormatString(300, 0, GetColor(255, 255, 255), "GameScene");
	// 必ずリセット！
	//SetDrawBright(255, 255, 255);
	SetDrawBlendMode(DX_BLENDMODE_MULA, 128);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(255, 255, 255), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← 必ずリセット！
	//DrawGrid();

	//RT3
	SetDrawScreen(m_RT3); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	m_player->Draw();
	m_enemyManager->Draw();
#ifdef _DEBUG
	CollisionManager::GetInstance().DebugDraw();
	
#endif

	//最終的に画面に描画する
	SetDrawScreen(DX_SCREEN_BACK); ClearDrawScreen();
	DrawGraph(0, 0, m_RT1, TRUE); // 背景
	DrawGraph(0, 0, m_RT2, TRUE); // エフェクト・文字
	DrawGraph(0, 0, m_RT3, TRUE); // キャラクター
}

void GameScene::FadeOutDraw()
{
}

void GameScene::DrawGrid()
{
	// 直線の始点と終点
	VECTOR startPos;
	VECTOR endPos;

	for (int z = -kGridRange; z <= kGridRange; z += 100)
	{
		startPos = VGet(-kGridRange, 0.0f, static_cast<float>(z));
		endPos = VGet(kGridRange, 0.0f, static_cast<float>(z));
		DrawLine3D(startPos, endPos, 0xff0000);
	}
	for (int x = -kGridRange; x <= kGridRange; x += 100)
	{
		startPos = VGet(static_cast<float>(x), 0.0f, -kGridRange);
		endPos = VGet(static_cast<float>(x), 0.0f, kGridRange);
		DrawLine3D(startPos, endPos, 0x0000ff);
	}
}

void GameScene::RockOnCamera()
{
	//問題点
	//・敵がいないときのことを考えていない
	// →敵がいないときはキャラクターの正面を向くようにする
	//・敵が死んでいるときのことを考えていない



	const auto& camera = m_cameraManager->GetHighestPriorityCamera();
	//dynamic_pointer_castでキャストする
	auto playerCamera = std::dynamic_pointer_cast<PlayerCamera>(camera);
	//キャストできなかったらreturn
	if (!playerCamera)return;

	auto& input = Input::GetInstance();

	bool isLockOn = playerCamera->GetIsLockOn();

	//いったんボタン押したらロックオンする
	if (input.IsTriggered("RB"))
	{
		//ロックオンしていないとき
		if (!isLockOn)
		{
			//ロックオンする
		//敵を持ってきて、プレイヤーの範囲内にいるやつを取得//一旦飛ばす
			const auto& enemies = m_enemyManager->GetEnemies();
			//ロックオンする敵を決める
			Vector3 rayVec = m_player->GetPos() - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
			rayVec.Normalize();
			//最小角度//Cos//最大Cosを求める
			float maxCos = -1.0f;
			for (auto& enemy : enemies)
			{
				//敵が死んでいたらスキップ//一旦飛ばし
				
				//敵の座標
				Vector3 enemyPos = enemy->GetPos();
				//カメラから敵までのベクトル
				Vector3 toEnemyVec = enemyPos - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
				//vecの初期化
				toEnemyVec.Normalize();

				//rayVecとtoEnemyVecの角度を求める//acosfじゃなくてcosで十分
				float cos = rayVec.Dot(toEnemyVec);
				//cosが最大のやつをロックオンする
				if (cos > maxCos)
				{
					maxCos = cos;
					//ロックオンする//敵をカメラに渡す
					playerCamera->SetLockOnEnemy(std::weak_ptr<EnemyBase>(enemy));
				}
			}
		}
		//ロックオンしているとき
		else
		{
			//ロックオンを解除する
			playerCamera->ReleaseLockOnEnemy();
		}

	}
}
