#include "GameScene.h"
#include "../Camera/CameraManager.h"
#include "../Camera/PlayerCamera.h"
#include "../Camera/MainCamera.h"
#include "Player.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"
#include "../Stage/Stage.h"
#include "../Stage/SkyBox.h"
#include "../Game.h"
#include "../System.h"
#include "../Input.h"
#include "EffekseerForDXLib.h"
#include "../Character/Enemy/EnemyManager.h"
#include "../UI/UIManager.h"
#include "../UI/PlayerHUD.h"
#include "../SubWindow/SubWindow.h"
#include "SceneController.h"
#include "GameOverScene.h"
#include "GameClearScene.h"
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


	constexpr int kLockOnCheckNum = 10;//ロックオンの検索ループ回数
}


GameScene::GameScene(SceneController& controller) :Scene(controller)
{
	//基底クラスに継承先のポインタをキャストして代入する
	m_updateFunc = static_cast<UpdateFunc_t>(&GameScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameScene::NormalDraw);

	//ステージの初期化
	m_stage = std::make_shared<Stage>();
	m_stage->Init();
	//プレイヤーの初期化
	m_player = std::make_shared<Player>();
	m_player->Init();
	//敵の初期化
	/*m_enemySwordman = std::make_shared<EnemySwordman>(std::weak_ptr<Player>(m_player));
	m_enemySwordman->Init();*/
	m_enemyManager = std::make_shared<EnemyManager>(std::weak_ptr<Player>(m_player));
	m_enemyManager->Init();

	m_cameraManager = std::make_shared<CameraManager>();
	//カメラの初期化
	m_cameraManager->Init(std::weak_ptr<Player>(m_player), std::weak_ptr<Stage>(m_stage));
	m_cameraManager->Update(m_player->GetPos());

	//スカイボックスの初期化
	m_skyBox = std::make_shared<SkyBox>();
	m_skyBox->Init(std::weak_ptr<Camera>(m_cameraManager->GetMainCamera()));

	//プレイヤーにカメラマネージャーの弱参照を渡す
	m_player->SetCameraManager(std::weak_ptr<CameraManager>(m_cameraManager));
	//プレイヤーにEnemyManagerの弱参照を渡す
	m_player->SetEnemyManager(std::weak_ptr<EnemyManager>(m_enemyManager));

	CollisionManager::GetInstance().Init();
	CollisionManager::GetInstance().SetStage(std::weak_ptr(m_stage));

	//レンダーターゲットの作成
	m_RT1 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	m_RT2 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	m_RT3 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	//シェーダーの作成
	m_ultShaderHandle = CreateShaderConstantBuffer(sizeof(UltParam));
	m_ultCBuff = static_cast<UltParam*>(GetBufferShaderConstantBuffer(m_ultShaderHandle));
	
	m_enemyPSH = LoadPixelShader("EnemyBlackPS.pso");

	//bone確かめ用
	int triangleType = MV1GetTriangleListVertexType(System::GetInstance().GetHandle(AsyncData::EnemyModel),0);


	std::vector<D3D_SHADER_MACRO> macros = {
		{"SKINMESH",""},//最後に空のマクロを入れる
		{"BUMPMAP",""},//最後に空のマクロを入れる
		{nullptr,nullptr}
	};

	m_enemyVSH = LoadVertexShaderWithMacro("Shader/MV1VertexShader.hlsl", macros);
	//m_enemyVSH = LoadVertexShader("EnemyBlack.pso");


	m_uiManager = std::make_shared<UIManager>();
	//UIの追加
	auto playerHUD = std::make_shared<PlayerHUD>();
	playerHUD->SetPlayer(std::weak_ptr<Player>(m_player));
	m_uiManager->AddUI(playerHUD);


	m_gHandle1 = LoadGraph("data/UI/blood_UI.png");
	m_gHandle2 = LoadGraph("data/UI/splash2_UI.png");
	m_gHandle3 = LoadGraph("data/UI/satu_UI.png");


}
GameScene::~GameScene()
{
	//Collisionをすべてクリア
	CollisionManager::GetInstance().Terminate();


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
	CheckLockOnCamera();
	//ロックオンカメラの敵の切り替え処理
	LockOnCameraInput();
	//ロックオンしている敵が死んだら、次の敵に切り替える
	CheckLockOnCameraEnemyDead();

	m_player->Update(*m_cameraManager->GetHighestPriorityCamera());
	m_enemyManager->Update();
	m_stage->Update();
	m_skyBox->Update();
	CollisionManager::GetInstance().Update();
	m_cameraManager->Update(m_player->GetPos());//カメラの更新をplayerの前からCollisionManagerの後に変更//何かバグるかも
	System::GetInstance().Update();
	m_uiManager->Update();

	//playerが死んでいたらゲームオーバーシーンに遷移
	if (m_player->GetIsDead())
	{
		m_controller.ChangeScene(std::make_shared<GameOverScene>(m_controller));
		return;
	}
	//すべての敵を倒したらゲームクリアシーンに遷移
	if (m_enemyManager->IsGetAllEnemiesDead(static_cast<int>(WaveNum::WaveSize) -1))
	{
		m_controller.ChangeScene(std::make_shared<GameClearScene>(m_controller));
		return;
	}
	//playerはそのwaveの敵を倒さないと次のwaveに進めない
	//spawnwaveがtrueでそのwaveのenemyAllDeadがfalseならそのwaveを進めない
	for(int i = 0; i < static_cast<int>(WaveNum::WaveSize); i++)
	{
		//spawnwaveがtrueでそのwaveのenemyAllDeadがfalseならそのwaveを進めない
		if (m_enemyManager->isSpawnedWave(i) && !m_enemyManager->IsGetAllEnemiesDead(i))
		{
			//プレイヤーの条件にセット
			m_player->SetLimitPlayerArea(i,true);
		}
		////もしどちらもtrueならplayerの条件をセットする
		if (m_enemyManager->isSpawnedWave(i) && m_enemyManager->IsGetAllEnemiesDead(i))
		{
			//プレイヤーの条件にセット
			m_player->SetLimitPlayerArea(i,false);
		}
	}

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
	m_skyBox->Draw();
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
	SetDrawScreen(m_RT2);
	SetBackgroundColor(0, 0, 0); 
	ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	//シェーダーで色をとって白くする
	//SetDrawBright(0, 0, 0);  // R=255, G=255, B=255	
	m_player->EffectDraw();
	//描画前に色を設定
	Effekseer_Sync3DSetting();;
	//ultの時だけ、ここに描画して、エフェクトを白くする
	//DrawEffekseer3D();
	DrawFormatString(300, 0, GetColor(255, 255, 255), "GameScene");


	//DrawGrid();

	//RT3
	SetDrawScreen(m_RT3); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	//シェーダーのセット
	MV1SetUseOrigShader(true);
	SetUsePixelShader(m_enemyPSH);
	SetUseVertexShader(m_enemyVSH);


	if (isUlt)
	{
		m_ultCBuff->ultAmount = 1.0f;
		UpdateShaderConstantBuffer(m_ultShaderHandle);
	}
	else
	{
		m_ultCBuff->ultAmount = 0.0f;
		UpdateShaderConstantBuffer(m_ultShaderHandle);
	}
	SetShaderConstantBuffer(m_ultShaderHandle, DX_SHADERTYPE_PIXEL, 5);

	m_enemyManager->Draw();
	//シェーダーの解除
	SetShaderConstantBuffer(-1, DX_SHADERTYPE_PIXEL, 5);

	SetUsePixelShader(-1);
	SetUseVertexShader(-1);
	MV1SetUseOrigShader(false);

	//UIの表示
	if (isUlt)
	{
		DrawRectRotaGraph(Game::kScreenWidth - 200, Game::kScreenHeight / 4, 0, 0, kGHX, kGHY, 0.8f, -DX_PI_F / 12, m_gHandle1, TRUE);
		DrawRectRotaGraph(Game::kScreenWidth / 15, Game::kScreenHeight - 150, 0, 0, kGH2X, kGH2Y, 1.0f, DX_PI_F / 4, m_gHandle2, TRUE);
		DrawRectRotaGraph(Game::kScreenWidth * 3 / 5, Game::kScreenHeight - 200, 0, 0, kGH3X, kGH3Y, 0.5f, 0.0f, m_gHandle3, TRUE);
	}
	m_player->Draw();
//#ifdef _DEBUG
	//CollisionManager::GetInstance().DebugDraw();
	//PlayerのHpのデバッグ表示
	
	//ロックオンしている敵のデバッグ表示
	auto targetEnemy = m_cameraManager->GetTargetEnemy();
	if (targetEnemy)
	{
		Vector3 enemyPos = targetEnemy->GetPos() + Vector3 (0,200,0);

		VECTOR enemyPos2D;;

		//スクリーン座標に変換
		enemyPos2D = ConvWorldPosToScreenPos(enemyPos.ToDxLibVector());

		// 画面内にいるときだけ描画
		if (enemyPos2D.z >= 0.0f && enemyPos2D.z <= 1.0f)
		{
			// ロックオンサークル
			DrawCircle(static_cast<int>(enemyPos2D.x), static_cast<int>(enemyPos2D.y), 20, GetColor(255, 255, 0), true);
		}
	}


//#endif
	m_cameraManager->Draw();
	m_uiManager->Draw();

	//最終的に画面に描画する
	SetDrawScreen(DX_SCREEN_BACK); ClearDrawScreen();
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	DrawGraph(0, 0, m_RT1, true); // 背景
	SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	DrawGraph(0, 0, m_RT2, true); // エフェクト・文字
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	DrawGraph(0, 0, m_RT3, true); // キャラクター
	//SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	// Effekseerはバックバッファ上で描画する（MakeScreenへの描画は非対応）
	m_cameraManager->ApplyCameraSettings();
	//SetDrawBlendMode(DX_BLENDMODE_MUL, 255);
	//DrawEffekseer3D();	
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
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

void GameScene::CheckLockOnCamera()
{
	//問題点
	//・敵がいないときのことを考えていない
	// →敵がいないときはキャラクターの正面を向くようにする



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
			//範囲をだんだん広げる//一旦3回繰り返す
			for (int i = 0; i < kLockOnCheckNum; i++)
			{
				for (auto& enemy : enemies)
				{
					//敵が死んでいたらスキップ//一旦飛ばし
					if (enemy->GetIsLifeZero())continue;

					Vector3 enemyPos = enemy->GetPos();
					Vector3 playerPos = m_player->GetPos();
					float toEnemyVecMag = (enemyPos - playerPos).Magnitude();
					//範囲内にいる敵を取得
					if (toEnemyVecMag < (m_player->GetCameraRockOnRange() * i))
					{
						enemiesInRange.push_back(enemy);
					}
				}
				//敵がいたらそこで抜ける
				if (!enemiesInRange.empty())
				{
					break;
				}
			}
			//範囲内に敵がいなかったらreturn
			if (enemiesInRange.empty())
			{
				return;
			}


			//ロックオンする敵を決める
			Vector3 rayVec = m_player->GetPos() - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
			rayVec = rayVec.Normalize();
			//最小角度//Cos//最大Cosを求める
			float maxCos = -1.0f;
			for (auto& enemy : enemiesInRange)
			{
				//敵の座標
				Vector3 enemyPos = enemy->GetPos();
				//カメラから敵までのベクトル
				Vector3 toEnemyVec = enemyPos - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
				//vecの初期化
				toEnemyVec = toEnemyVec.Normalize();

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
			//ラープを切る
			mainCamera->SetLerp(false);
			mainCamera->SetSlerp(false);
			m_cameraManager->SetNextCameraPriority(Camera::Type::PlayerCamera, false, false);
		}

	}
}

void GameScene::LockOnCameraInput()
{
	//左右の入力があったときにそれを切り替える処理をする

	const auto& camera = m_cameraManager->GetHighestPriorityCamera();

	auto& input = Input::GetInstance();

	//ウルトカメラ中だったらreturn;
	if (camera->GetCameraType() == Camera::Type::UltCamera)return;

	auto mainCamera = m_cameraManager->GetMainCamera();

	bool isLockOn = mainCamera->GetIsLockOn();
	//playerがいなかったらreturnする
	if (!m_player)return;
	//ロックオンしていなかったらreturnする
	if (!isLockOn)return;

	//右スティックの入力がなかったらreturnする
	bool isTriggerdRightStickX = input.IsTriggeredRightStickInputX();
	if (!isTriggerdRightStickX)return;

	//左右の入力があったとき、少し大きめな範囲で敵を取り、cosの値で、その敵から入力方向の敵を選ぶ
	auto rightStick = input.GetRightStickInput();
	if (rightStick.x != 0.0f)
	{
		//ロックオンしている敵を取得
		auto lockedEnemy = m_cameraManager->GetTargetEnemy();
		if (!lockedEnemy)return;
		//ロックオンしている敵の座標
		Vector3 lockedEnemyPos = lockedEnemy->GetPos();


		//敵を持ってきて、プレイヤーの範囲内にいるやつを取得//一旦飛ばす
		const auto& enemies = m_enemyManager->GetEnemies();
		//範囲指定をする
		std::vector<std::shared_ptr<EnemyBase>> enemiesInRange;
		for (auto& enemy : enemies)
		{
			//敵が死んでいたらスキップ
			if (enemy->GetIsLifeZero())continue;
			//ロックオンしている敵はスキップ
			if (enemy == lockedEnemy)continue;

			Vector3 enemyPos = enemy->GetPos();
			Vector3 playerPos = m_player->GetPos();
			float toEnemyVecMag = (enemyPos - playerPos).Magnitude();
			//範囲内にいる敵を取得
			if (toEnemyVecMag < (m_player->GetCameraRockOnRange() * 3))//3倍の範囲で取得
			{
				enemiesInRange.push_back(enemy);
			}
		}
		//範囲内に敵がいなかったらreturn
		if (enemiesInRange.empty())
		{
			return;
		}
		//ターゲットエネミーとカメラのベクトルと、他の敵たちとのカメラベクトルのcosの値で決める
		Vector3 rayVec = lockedEnemyPos - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
		rayVec = rayVec.Normalize();
		std::vector<std::shared_ptr<EnemyBase>> enemiesInRangeForward;
		//Cosが正のものを取り出す
		for (auto& enemy : enemiesInRange)
		{
			//敵の座標
			Vector3 enemyPos = enemy->GetPos();
			//カメラから敵までのベクトル
			Vector3 toEnemyVec = enemyPos - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
			//vecの初期化
			toEnemyVec = toEnemyVec.Normalize();

			//rayVecとtoEnemyVecの角度を求める
			float cos = rayVec.Dot(toEnemyVec);
			//cosが最大のやつをロックオンする
			if (cos > 0)
			{
				//前方の敵に入れる
				enemiesInRangeForward.push_back(enemy);
			}
		}
		//範囲内に敵がいなかったらreturn
		if (enemiesInRangeForward.empty())
		{
			return;
		}
		//前方にいる敵と、ターゲットベクトルの90度回転ベクトルとの内積で、右にいる敵か左にいる敵かを決める
		Vector3 upVec = Vector3(0, 1, 0);
		//外積で右方向のベクトルを求める
		Vector3 rightVec = rayVec.Cross(upVec) * -1;
		rightVec = rightVec.Normalize();//正規化する

		float minPositiveDot = 1.0f;//正の内積の最小値を初期化
		float minNegativeDot = -1.0f;//負の内積の最大値を初期化

		for (auto& enemy : enemiesInRangeForward)
		{
			//敵の座標
			Vector3 enemyPos = enemy->GetPos();
			//カメラから敵までのベクトル
			Vector3 toEnemyVec = enemyPos - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
			//vecの初期化
			toEnemyVec = toEnemyVec.Normalize();
			//右方向のベクトルとtoEnemyVecの内積を求める
			float dot = rightVec.Dot(toEnemyVec);
			//右スティックの入力が正のとき、内積が正で最も0に近い敵を選ぶ
			if (rightStick.x > 0 && dot > 0 && dot < minPositiveDot)
			{
				minPositiveDot = dot;
				m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player), std::weak_ptr<EnemyBase>(enemy));
			}
			//右スティックの入力が負のとき、内積が負で最も0に近い敵を選ぶ
			else if (rightStick.x < 0 && dot < 0 && dot > minNegativeDot)
			{
				minNegativeDot = dot;
				m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player), std::weak_ptr<EnemyBase>(enemy));
			}
		}


	}


}

void GameScene::CheckLockOnCameraEnemyDead()
{
	//ロックオンしている敵が死んだら、次の敵に切り替える処理をする
	//最も近くの敵

	const auto& camera = m_cameraManager->GetHighestPriorityCamera();

	//ウルトカメラ中だったらreturn;
	if (camera->GetCameraType() == Camera::Type::UltCamera)return;

	auto mainCamera = m_cameraManager->GetMainCamera();

	bool isLockOn = mainCamera->GetIsLockOn();
	//playerがいなかったらreturnする
	if (!m_player)return;
	//ロックオンしていなかったらreturnする
	if (!isLockOn)return;
	//ロックオンしている敵を取得
	auto lockedEnemy = m_cameraManager->GetTargetEnemy();
	//ロックオンしている敵がいなかったら
	if (!lockedEnemy)
	{
		assert(false && "ロックオンしている敵がいません");
		return;
	}
	else
	{
		//ロックオンしている敵が死んでいなかったらreturnする
		if (!lockedEnemy->GetIsLifeZero())return;
		//ロックオンしている敵の座標
		Vector3 lockedEnemyPos = lockedEnemy->GetPos();

		//敵を持ってきて、プレイヤーの範囲内にいるやつを取得
		const auto& enemies = m_enemyManager->GetEnemies();
		//範囲指定をする
		std::vector<std::shared_ptr<EnemyBase>> enemiesInRange;
		for (auto& enemy : enemies)
		{
			//敵が死んでいたらスキップ
			if (enemy->GetIsLifeZero())continue;
			//ロックオンしている敵はスキップ
			if (enemy == lockedEnemy)continue;

			Vector3 enemyPos = enemy->GetPos();
			Vector3 playerPos = m_player->GetPos();
			float toEnemyVecMag = (enemyPos - playerPos).Magnitude();
			//範囲内にいる敵を取得
			if (toEnemyVecMag < (m_player->GetCameraRockOnRange() * 3))//3倍の範囲で取得
			{
				enemiesInRange.push_back(enemy);
			}
		}
		//範囲内に敵がいなかったらロックオンを解除する
		if (enemiesInRange.empty())
		{
			//ロックオンを解除する
			m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player));
			mainCamera->SetLockOn(false);
			return;
		}
		//ターゲットエネミーとカメラのベクトルと、他の敵たちとのカメラベクトルのcosの値で決める
		Vector3 rayVec = lockedEnemyPos - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
		rayVec = rayVec.Normalize();
		//最小角度//Cos//最大Cosを求める
		float maxCos = -1.0f;
		for (auto& enemy : enemiesInRange)
		{
			//敵の座標
			Vector3 enemyPos = enemy->GetPos();
			//カメラから敵までのベクトル
			Vector3 toEnemyVec = enemyPos - m_cameraManager->GetHighestPriorityCamera()->GetCameraPos();
			//vecの初期化
			toEnemyVec = toEnemyVec.Normalize();

			//rayVecとtoEnemyVecの角度を求める
			float cos = rayVec.Dot(toEnemyVec);
			if (cos > maxCos)
			{
				maxCos = cos;
				//敵をカメラに渡す
				m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player), std::weak_ptr<EnemyBase>(enemy));
			}

			//誰もいなかったらロックオンを解除する
			if (maxCos == -1.0f)
			{
				//ロックオンを解除する
				m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player));
				mainCamera->SetLockOn(false);
			}
		}
	}

}

int GameScene::LoadVertexShaderWithMacro(const std::string& filePath, std::vector<D3D_SHADER_MACRO>& macros)
{

	//BLOB=Binary Learge OBjectの略
		//おそらく、「なんかよくわからない塊」という意味のBlobとかけている
	ID3DBlob* pVSShader = nullptr;//バイナリの塊(形式がよくわからない:中身はコンパイル済みシェーダ)→vso,psoにあたる
	ID3DBlob* pMsg = nullptr;//バイナリの塊(エラーを起こしたときのエラーメッセージが入る)
	//resultはただのlong型の数値ですが、この値によって何が起きてるかわかるようになっている。
	std::wstring wstr(filePath.begin(), filePath.end());
	HRESULT result = D3DCompileFromFile(wstr.c_str(),
		macros.data(),
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0, 0, &pVSShader, &pMsg);

	//失敗したらエラー出力
	//if (result != S_OK) {
	if (FAILED(result)) {
		//メッセージをVisualStudio等の「出力」に出力する
		auto size = pMsg->GetBufferSize();
		std::string strMsg;
		strMsg.resize(size);
		std::copy_n((char*)pMsg->GetBufferPointer(), size, strMsg.data());
		OutputDebugStringA(strMsg.c_str());//「出力」にエラーの内容を出力する
		assert(0);
	}//ここまでDirectXの関数です

	//最後のこれだけDXライブラリの関数です
	//成功したら、頂点バッファオブジェクトを作る(スキンメッシュ)
	int vsH = LoadVertexShaderFromMem(pVSShader->GetBufferPointer(), pVSShader->GetBufferSize());
	assert(vsH >= 0);
	//DirectX系のオブジェクトは解放がちょっとややこしくてdeleteではなく、Releaseメソッドを
	//呼び出します。
	pVSShader->Release();
	if (pMsg != nullptr) {
		pMsg->Release();
	}

	return vsH;
}
