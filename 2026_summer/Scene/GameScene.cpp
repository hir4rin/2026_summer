#include "GameScene.h"
#include "../Camera/CameraManager.h"
#include "../Camera/PlayerCamera.h"
#include "../Camera/MainCamera.h"
#include "Player.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"
#include "../Stage/Stage.h"
#include "../Game.h"
#include "../System.h"
#include "../Input.h"
#include "EffekseerForDXLib.h"
#include "../Managers/EnemyManager.h"
#include "../SubWindow/SubWindow.h"

namespace
{
	constexpr int kGridRange = 2400;//グリッドのサイズ
	


	constexpr int kGHX = 782;
	constexpr int kGHY = 639;
	constexpr int kGH2X = 1156;
	constexpr int kGH2Y = 758;
	/*constexpr int kGH2X = 1262;
	constexpr int kGH2Y = 910;*/
	constexpr int kGH3X = 687;
	constexpr int kGH3Y = 598;
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


	m_cameraManager = std::make_shared<CameraManager>();
	//カメラの初期化
	m_cameraManager->Init(std::weak_ptr<Player>(m_player));
	m_cameraManager->Update(m_player->GetPos());

	m_stage = std::make_shared<Stage>();
	CollisionManager::GetInstance().Init();
	//レンダーターゲットの作成
	m_RT1 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	m_RT2 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	m_RT3 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);


	 m_gHandle1 = LoadGraph("data/UI/blood_UI.png");
	 m_gHandle2 = LoadGraph("data/UI/splash2_UI.png");
	 m_gHandle3 = LoadGraph("data/UI/satu_UI.png");


}
GameScene::~GameScene()
{
	DeleteGraph(m_gHandle1);
	DeleteGraph(m_gHandle2);
	DeleteGraph(m_gHandle3);
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
	System::GetInstance().Update();
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

	bool isUlt = System::GetInstance().GetIsUltimating();


	//RT1
	SetDrawScreen(m_RT1); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	//赤くする
	m_stage->Draw();
	if (isUlt)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
		DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(255, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← 必ずリセット！
	}
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

	
	//DrawGrid();

	//RT3
	SetDrawScreen(m_RT3); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	m_enemyManager->Draw();

	//UIの表示
	if (isUlt)
	{
		DrawRectRotaGraph(Game::kScreenWidth - 200, Game::kScreenHeight / 4, 0, 0, kGHX, kGHY, 0.8f, -DX_PI_F / 12, m_gHandle1, TRUE);
		DrawRectRotaGraph(Game::kScreenWidth / 15, Game::kScreenHeight - 150, 0, 0, kGH2X, kGH2Y, 1.0f, DX_PI_F / 4, m_gHandle2, TRUE);
		DrawRectRotaGraph(Game::kScreenWidth * 3 / 5, Game::kScreenHeight - 200, 0, 0, kGH3X, kGH3Y, 0.5f, 0.0f, m_gHandle3, TRUE);
	}
	m_player->Draw();
#ifdef _DEBUG
	CollisionManager::GetInstance().DebugDraw();
#endif
	m_cameraManager->Draw();

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
	//// 直線の始点と終点
	//VECTOR startPos;
	//VECTOR endPos;

	//for (int z = -kGridRange; z <= kGridRange; z += 100)
	//{
	//	startPos = VGet(-kGridRange, 0.0f, static_cast<float>(z));
	//	endPos = VGet(kGridRange, 0.0f, static_cast<float>(z));
	//	DrawLine3D(startPos, endPos, 0xff0000);
	//}
	//for (int x = -kGridRange; x <= kGridRange; x += 100)
	//{
	//	startPos = VGet(static_cast<float>(x), 0.0f, -kGridRange);
	//	endPos = VGet(static_cast<float>(x), 0.0f, kGridRange);
	//	DrawLine3D(startPos, endPos, 0x0000ff);
	//}
}

void GameScene::RockOnCamera()
{
	//問題点
	//・敵がいないときのことを考えていない
	// →敵がいないときはキャラクターの正面を向くようにする
	//・敵が死んでいるときのことを考えていない



	const auto& camera = m_cameraManager->GetHighestPriorityCamera();

	auto& input = Input::GetInstance();

	//ウルトカメラ中だったらreturn;
	if (camera->GetCameraType() == Camera::Type::UltCamera)return;

	auto mainCamera = m_cameraManager->GetMainCamera();

	bool isLockOn = mainCamera->GetIsLockOn();
	//playerがいなかったらreturnする
	if (!m_player)return;

	//いったんボタン押したらロックオンする
	if (input.IsTriggered("RB"))
	{
		//ロックオンしていないとき
		if (!isLockOn)
		{
			//ロックオンする
		//敵を持ってきて、プレイヤーの範囲内にいるやつを取得//一旦飛ばす
			const auto& enemies = m_enemyManager->GetEnemies();
			//範囲指定をする
			std::vector<std::shared_ptr<EnemyBase>> enemiesInRange;
			for (auto& enemy : enemies)
			{
				Vector3 enemyPos = enemy->GetPos();
				Vector3 playerPos = m_player->GetPos();
				float toEnemyVec = (enemyPos - playerPos).Magnitude();
				//範囲内にいる敵を取得
				if (toEnemyVec < m_player->GetCameraRockOnRange())
				{
					enemiesInRange.push_back(enemy);
				}

			}
			if(enemiesInRange.empty())
			{
				//範囲内に敵がいなかったらreturn
				return;
			}

			//ロックオンする敵を決める
			Vector3 rayVec = m_player->GetPos() - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
			rayVec.Normalize();
			//最小角度//Cos//最大Cosを求める
			float maxCos = -1.0f;
			for (auto& enemy : enemiesInRange)
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
					//ここでCameraManagerに渡して、いろいろに渡す
					m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player), std::weak_ptr<EnemyBase>(enemy));
					mainCamera->SetLockOn(true);

					
				}
			}
		}
		//ロックオンしているとき
		else
		{
			//ロックオンを解除する
			m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player));
			
			mainCamera->SetLockOn(false);
		}

	}
}
