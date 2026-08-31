#include "GameScene.h"
#include "../Camera/CameraManager.h"
#include "../Camera/CameraState/CameraStateBase.h"
#include "../Camera/LockOnManager.h"
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
#include "../Character/Enemy/Boss/BossEnemy.h"
#include "../UI/UIManager.h"
#include "../UI/PlayerHUD.h"
#include "../SubWindow/SubWindow.h"
#include "SceneController.h"
#include "GameOverScene.h"
#include "GameClearScene.h"
#include "GameResult.h"
#include "StageClearPoPScene.h"
#include "PauseScene.h"
#include "StageClearPopScene.h"
#include "../imguiApp.h"

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
	constexpr int kFps = 60;//fpsを60に固定して動かしているので、クリアタイムの算出に使う

	constexpr float kFlashDuration = 12.0f;

	constexpr int kFadeFrame = 20;//フェードイン・フェードアウトにかけるフレーム数
}


GameScene::GameScene(SceneController& controller) :Scene(controller)
{
	//基底クラスに継承先のポインタをキャストして代入する//黒画面からフェードインしてスタートする
	m_updateFunc = static_cast<UpdateFunc_t>(&GameScene::FadeInUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameScene::FadeInDraw);

	//ステージの初期化
	m_stage = std::make_shared<Stage>();
	m_stage->Init();
	m_stage->GameInit();
	//プレイヤーの初期化
	m_player = std::make_shared<Player>();
	m_player->Init();
	//敵の初期化
	/*m_enemySwordman = std::make_shared<EnemySwordman>(std::weak_ptr<Player>(m_player));
	m_enemySwordman->Init();*/
	m_enemyManager = std::make_shared<EnemyManager>(std::weak_ptr<Player>(m_player));
	m_enemyManager->Init();
	//ボスはEnemyManagerが最終フェーズ到達時に生成する(EnemyManager::SpawnBoss参照)

	//カメラマネージャーの生成
	m_cameraManager = std::make_shared<CameraManager>();

	//ロックオンマネージャーを作る
	m_lockOnManager = std::make_shared<LockOnManager>();
	m_cameraManager->SetLockOnCamera(m_lockOnManager);
	m_player->SetLockOnManager(m_lockOnManager);
	//カメラの初期化
	m_cameraManager->Init(std::weak_ptr<Player>(m_player), std::weak_ptr<Stage>(m_stage));
	m_cameraManager->Update(m_player->GetPos());

	//スカイボックスの初期化
	m_skyBox = std::make_shared<SkyBox>();
	m_skyBox->Init(std::weak_ptr<CameraManager>(m_cameraManager));

	//プレイヤーにカメラマネージャーの弱参照を渡す
	m_player->SetCameraManager(std::weak_ptr<CameraManager>(m_cameraManager));
	//プレイヤーにEnemyManagerの弱参照を渡す
	m_player->SetEnemyManager(std::weak_ptr<EnemyManager>(m_enemyManager));
	//EnemyManagerにカメラマネージャーの弱参照を渡す(フェーズ演出でのカメラ切り替えに使う)
	m_enemyManager->SetCameraManager(std::weak_ptr<CameraManager>(m_cameraManager));




	CollisionManager::GetInstance().Init();
	CollisionManager::GetInstance().SetStage(std::weak_ptr(m_stage));

	//レンダーターゲットの作成
	m_RT1 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	m_RT2 = MakeScreen(Game::kScreenWidth, Game::kScreenHeight, true);
	//シェーダーの作成
	m_ultShaderHandle = CreateShaderConstantBuffer(sizeof(UltParam));
	m_ultCBuff = static_cast<UltParam*>(GetBufferShaderConstantBuffer(m_ultShaderHandle));

	m_enemyPSH = LoadPixelShader("EnemyBlackPS.pso");

	//bone確かめ用
	int triangleType = MV1GetTriangleListVertexType(System::GetInstance().GetHandle(AsyncData::EnemyModel), 0);


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

	//BGMを流す
	System::GetInstance().GetSoundManager().PlayBgm("BattleBGM");
}
GameScene::~GameScene()
{
	Terminate();
	
}

void GameScene::Update()
{
	(this->*m_updateFunc)();
}

void GameScene::FadeInUpdate()
{
	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	m_fadeCount++;
	if (m_fadeCount >= kFadeFrame)
	{
		//フェードインが終わったので、通常のゲーム動作に戻す
		m_fadeCount = 0;
		m_updateFunc = static_cast<UpdateFunc_t>(&GameScene::NormalUpdate);
		m_drawFunc = static_cast<DrawFunc_t>(&GameScene::NormalDraw);
	}
}

void GameScene::NormalUpdate()
{
	//System::GetInstance().SetTimeScale(0.1f);


	auto& input = Input::GetInstance();
	if (input.IsTriggered("Start"))
	{
		//ポーズシーンを積む
		//角度を計算させて保存
		m_cameraManager->GetActiveCamera()->Exit();
		m_cameraManager->SetPhotoCamera();
		m_controller.PushScene(std::make_shared<PauseScene>(m_controller));
		return;
	}

	//フォトモードだったら、カメラのみを動かせるようにする
	if (System::GetInstance().GetPhotoMode())
	{

		m_cameraManager->UpdatePhotoCamera();


		if (input.IsTriggered("Y"))
		{
			//このタイミングではまだ今フレームの描画が終わっていないため、
			//フラグだけ立てて実際の保存はDrawの最後で行う
			m_requestScreenshot = true;
		}

		//もう一度スタートを押したらpushSceneに行く
		if (input.IsTriggered("Start"))
		{
			//ポーズシーンを積む
			m_controller.PushScene(std::make_shared<PauseScene>(m_controller));
			return;
		}
		return;
	}


	//ロックオンするか
	CheckLockOnCamera();
	//ロックオンカメラの敵の切り替え処理
	LockOnCameraInput();
	//ロックオンしている敵が死んだら、次の敵に切り替える
	CheckLockOnCameraEnemyDead();

	m_player->Update(*m_cameraManager->GetHighestPriorityCamera());
	m_enemyManager->Update();
	//ボスは最終フェーズに到達するまで存在しないのでnullチェックする
	if (auto boss = m_enemyManager->GetBoss())
	{
		boss->Update();
	}
	m_stage->Update();
	m_skyBox->Update();
	CollisionManager::GetInstance().Update();
	m_cameraManager->Update(m_player->GetPos());//カメラの更新をplayerの前からCollisionManagerの後に変更//何かバグるかも
	System::GetInstance().Update();
	m_uiManager->Update();

	m_playFrameCount++;//リザルトのクリアタイム集計用

	//playerが死んでいたら、フェードアウトしてからゲームオーバーシーンに遷移
	if (m_player->GetIsDead())
	{
		m_fadeCount = 0;
		m_fadeOutDestination = FadeOutDestination::GameOver;
		m_updateFunc = static_cast<UpdateFunc_t>(&GameScene::FadeOutUpdate);
		m_drawFunc = static_cast<DrawFunc_t>(&GameScene::FadeOutDraw);
		return;
	}
	//すべての敵を倒したら、フェードアウトしてからゲームクリアシーンに遷移
	if (m_enemyManager->IsGetAllEnemiesDead(static_cast<int>(WaveNum::WaveSize) - 1))
	{
		//リザルトの集計
		m_pendingResult.clearTime = static_cast<float>(m_playFrameCount) / kFps;
		m_pendingResult.totalDamage = m_player->GetTotalDamageDealt();
		m_pendingResult.comboCount = m_player->GetTotalHitCount();
		m_pendingResult.damageTakenCount = m_player->GetDamageTakenCount();

		m_fadeCount = 0;
		m_fadeOutDestination = FadeOutDestination::GameClear;
		m_updateFunc = static_cast<UpdateFunc_t>(&GameScene::FadeOutUpdate);
		m_drawFunc = static_cast<DrawFunc_t>(&GameScene::FadeOutDraw);
		return;
	}
	//playerはそのwaveの敵を倒さないと次のwaveに進めない
	//spawnwaveがtrueでそのwaveのenemyAllDeadがfalseならそのwaveを進めない
	for (int i = 0; i < static_cast<int>(WaveNum::WaveSize); i++)
	{
		//spawnwaveがtrueでそのwaveのenemyAllDeadがfalseならそのwaveを進めない
		if (m_enemyManager->isSpawnedWave(i) && !m_enemyManager->IsGetAllEnemiesDead(i))
		{
			//プレイヤーの条件にセット
			m_player->SetLimitPlayerArea(i, true);
		}
		////もしどちらもtrueならplayerの条件をセットする
		if (m_enemyManager->isSpawnedWave(i) && m_enemyManager->IsGetAllEnemiesDead(i))
		{
			//プレイヤーの条件にセット
			m_player->SetLimitPlayerArea(i, false);
		}
	}
	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();
	if (System::GetInstance().GetIsLastHitEventPlaying() && !m_isLastHitPlayingTrigger)
	{
		//いずれコルーチンみたいな感じで30F後に起動みたいにする(今のところそんな難しくないイメージのやつ)
		m_lasthitEventDuration = 0.0f;

		//血殺(必殺技ゲージ)を演出用に満タンにする
		m_player->MaxUltGauge();

		m_isLastHitPlayingTrigger = true;
	}

	if (System::GetInstance().GetIsLastHitEventPlaying())
	{
		m_lasthitEventDuration++;
		if (m_lasthitEventDuration == 300)
		{
			//もとに戻す
			System::GetInstance().SetTimeScale(1.0f);

			//playerをセット
			m_player->SetPos(Vector3(328.66f, 0.0f, 6903.45f));
			m_player->ForceIdleState();
			m_player->SetRotY(0.0f);//TODO: 演出に合わせて向きを調整する

			// Record セット、animation開始
			Input::GetInstance().StartRecord(CreateLastHitEventInputRecord());

			//ボスをセット、アニメーション開始
			m_enemyManager->GetBoss()->RestartDeadState();
			//ボスをスポーンした地点にセット
			m_enemyManager->ResetBossToSpawnPos();
			//カメラをセット
			m_cameraManager->ChangeStateFromScene(CameraManager::CameraStateName::FinishingFirstCamera);
		}

		//最後のカメラかつ
		//if (m_cameraManager->GetActiveCamera()->GetCameraType() == CameraStateBase::Type::FinishingSecondCamera)
		{
			//playerの血殺のアニメーションが終わるタイミング
			if (m_lasthitEventDuration == 640)
			{
				input.StopRecord();
				System::GetInstance().SetIsLastHitEventPlaying(false);
				//リザルトの集計
				GameResult result;
				result.clearTime = static_cast<float>(m_playFrameCount) / kFps;
				result.totalDamage = m_player->GetTotalDamageDealt();
				result.comboCount = m_player->GetTotalHitCount();
				result.damageTakenCount = m_player->GetDamageTakenCount();

				//ポーズシーンを積む
				m_controller.PushScene(std::make_shared<StageClearPoPScene>(m_controller, result));
				return;
			}
		}

	}
}

void GameScene::FadeOutUpdate()
{
	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	m_fadeCount++;
	if (m_fadeCount >= kFadeFrame)
	{
		switch (m_fadeOutDestination)
		{
		case FadeOutDestination::GameOver:
			m_controller.ChangeScene(std::make_shared<GameOverScene>(m_controller));
			return;
		case FadeOutDestination::GameClear:
			m_controller.ChangeScene(std::make_shared<GameClearScene>(m_controller, m_pendingResult));
			return;
		default:
			break;
		}
	}
}

void GameScene::Draw()
{
	(this->*m_drawFunc)();
}

void GameScene::FadeInDraw()
{
	NormalDraw();

	//だんだん透明にしていく
	int alpha = 255 * (kFadeFrame - m_fadeCount) / kFadeFrame;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameScene::NormalDraw()
{
	//UltDrawを作ったほうがいいかも
	//UI描画や、必殺技時のエフェクトを白くするようにレンダーターゲットを使う
	//map,effectにシェーダーをかける

	SetDrawScreen(DX_SCREEN_BACK); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();

	bool isUlt = System::GetInstance().GetIsUltimating();

	m_skyBox->Draw();
	m_stage->Draw();
	//赤くする
	if (isUlt)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
		DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(255, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← 必ずリセット！
	}

	DrawFormatString(300, 0, GetColor(255, 255, 255), "GameScene");

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

	//必殺技UIの表示//レンダーターゲット
	if (isUlt)
	{
		//ラストヒットイベント中は、配置違いの血殺UIを表示する
		if (m_cameraManager->GetActiveCamera()->GetCameraType() == CameraStateBase::Type::FinishingFirstCamera
			|| m_cameraManager->GetActiveCamera()->GetCameraType() == CameraStateBase::Type::FinishingSecondCamera)
		{
			DrawLastHitKessatsuUI();
		}
		//通常時
		else
		{
			DrawUltKessatsuUI();
		}
	}
	
	m_player->Draw();
	//ボスは最終フェーズに到達するまで存在しないのでnullチェックする
	if (auto boss = m_enemyManager->GetBoss())
	{
		boss->Draw();
	}
#ifdef _DEBUG
	CollisionManager::GetInstance().DebugDraw();
	//PlayerのHpのデバッグ表示

	//ロックオンしている敵のデバッグ表示
	auto targetEnemy = m_cameraManager->GetTargetEnemy();
	if (targetEnemy)
	{
		Vector3 enemyPos = targetEnemy->GetPos() + Vector3(0, 200, 0);

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


	m_cameraManager->Draw();
	if (System::GetInstance().GetPhotoMode())
	{
		//カメラ座標・角度・注視点までの距離をいじれるデバッグウィンドウを表示する
		//現在のMainCamera(実際に画面に反映されているカメラ)の座標・注視点を渡している
		//(ウィンドウ内の"Current Pos/Target"表示や、"Sync From Current"ボタンで使われる)
		imguiApp::GetInstance().DrawCameraDebugWindow(
			m_cameraManager->GetPhotoCameraPos(),
			m_cameraManager->GetPhotoCameraTarget());
	}

#endif
	if (!System::GetInstance().GetPhotoMode() && !System::GetInstance().GetIsLastHitEventPlaying())
	{
		m_uiManager->Draw();
	}
	//エフェクトの描画
	DrawEffekseer3D();

	//フォトモードでスクリーンショットが要求されていたら、全描画完了後に保存する
	if (m_requestScreenshot)
	{
		SaveDrawScreenToPNG(0, 0, Game::kScreenWidth, Game::kScreenHeight, "data/SaveData/screenShot.png");
		m_requestScreenshot = false;
		m_flashTimer = kFlashDuration;
		System::GetInstance().GetSoundManager().PlaySE("CameraSyatterSE");
	}

	if (m_flashTimer > 0)
	{
		int alpha = 255 * m_flashTimer / kFlashDuration; //残り時間に比例して薄くなる
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		m_flashTimer--;


	}

	//フォトモード中は十字を表示する//保存画像に写り込まないよう、スクリーンショット保存より後に描画する
	if (System::GetInstance().GetPhotoMode())
	{
		DrawPhotoModeCrosshair();
	}
}

void GameScene::FadeOutDraw()
{
	NormalDraw();

	//だんだん暗くしていく
	int alpha = 255 * m_fadeCount / kFadeFrame;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
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



	auto& input = Input::GetInstance();

	//ウルトカメラ中だったらreturn;
	if (m_cameraManager->GetActiveCamera()->GetCameraType() == CameraStateBase::Type::UltCamera)return;
	if (System::GetInstance().GetIsLastHitEventPlaying())return;

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
					m_cameraManager->ChangeStateFromScene(CameraManager::CameraStateName::LockOnCamera);
					m_lockOnManager->SetTargetEnemy(enemy);
					mainCamera->SetLockOn(true);
					m_cameraManager->SetLockOn(true);


				}
			}
		}
		//ロックオンしているとき
		else
		{
			//ロックオンを解除する
			m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player));
			m_lockOnManager->SetTargetEnemy(std::weak_ptr<EnemyBase> {});
			m_cameraManager->ChangeStateFromScene(CameraManager::CameraStateName::PlayerCaemra);
			mainCamera->SetLockOn(false);
			m_cameraManager->SetLockOn(false);
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

	auto& input = Input::GetInstance();

	//ウルトカメラ中だったらreturn;
	if (m_cameraManager->GetActiveCamera()->GetCameraType() == CameraStateBase::Type::UltCamera)return;
	if (System::GetInstance().GetIsLastHitEventPlaying())return;

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
				m_lockOnManager->SetTargetEnemy(std::weak_ptr<EnemyBase>(enemy));
			}
			//右スティックの入力が負のとき、内積が負で最も0に近い敵を選ぶ
			else if (rightStick.x < 0 && dot < 0 && dot > minNegativeDot)
			{
				minNegativeDot = dot;
				m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player), std::weak_ptr<EnemyBase>(enemy));
				m_lockOnManager->SetTargetEnemy(std::weak_ptr<EnemyBase>(enemy));
			}
		}


	}


}

void GameScene::CheckLockOnCameraEnemyDead()
{
	//ロックオンしている敵が死んだら、次の敵に切り替える処理をする
	//最も近くの敵

	//ウルトカメラ中だったらreturn;
	if (m_cameraManager->GetActiveCamera()->GetCameraType() == CameraStateBase::Type::UltCamera)return;
	if (System::GetInstance().GetIsLastHitEventPlaying())return;

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
			m_lockOnManager->SetTargetEnemy(std::weak_ptr<EnemyBase>{});
			mainCamera->SetLockOn(false);
			m_cameraManager->SetLockOn(false);
			m_cameraManager->ChangeStateFromScene(CameraManager::CameraStateName::PlayerCaemra);
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
				m_lockOnManager->SetTargetEnemy(std::weak_ptr<EnemyBase>(enemy));

			}

			//誰もいなかったらロックオンを解除する
			if (maxCos == -1.0f)
			{
				//ロックオンを解除する
				m_cameraManager->SetWeakRef(std::weak_ptr<Player>(m_player));
				m_lockOnManager->SetTargetEnemy(std::weak_ptr<EnemyBase>{});
				mainCamera->SetLockOn(false);
				m_cameraManager->SetLockOn(false);
				m_cameraManager->ChangeStateFromScene(CameraManager::CameraStateName::PlayerCaemra);
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

InputRecord GameScene::CreateLastHitEventInputRecord()
{
	//ラストヒットイベント中、プレイヤーを仮想操作するための入力データ
	//中の数値(durationFramesやpressedButtons)は仮なので、あとで調整する
	InputRecord record;
	record.push_back({ 10,{} });
	record.push_back({ 1, {"X"} });//1F目だけXボタンを押したことにする
	record.push_back({ 190, {} });//そのあとは空入力(何も押していない)にする
	record.push_back({ 1, {"LB","Y"}});
	record.push_back({ 999,{} });
	return record;
}

void GameScene::DrawUltKessatsuUI()
{
	//必殺技UIの表示//レンダーターゲット
	SetDrawScreen(m_RT1); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	DrawRectRotaGraph(Game::kScreenWidth - 200, Game::kScreenHeight / 4, 0, 0, kGHX, kGHY, 0.8f, -DX_PI_F / 12, m_gHandle1, TRUE);
	DrawRectRotaGraph(Game::kScreenWidth / 15, Game::kScreenHeight - 150, 0, 0, kGH2X, kGH2Y, 1.0f, DX_PI_F / 4, m_gHandle2, TRUE);
	DrawRectRotaGraph(Game::kScreenWidth * 3 / 5, Game::kScreenHeight - 200, 0, 0, kGH3X, kGH3Y, 0.5f, 0.0f, m_gHandle3, TRUE);

	SetDrawScreen(DX_SCREEN_BACK);
	m_cameraManager->ApplyCameraSettings();
	DrawGraph(0, 0, m_RT1, true);
}

void GameScene::DrawPhotoModeCrosshair()
{
	constexpr int kCrosshairLength = 12;//十字1本あたりの長さ
	constexpr int kCrosshairGap = 4;//中心のすきま
	int color = GetColor(255, 255, 255);
	int centerX = Game::kScreenWidth / 2;
	int centerY = Game::kScreenHeight / 2;

	//横線(中心にすきまをあけて2本)
	DrawLine(centerX - kCrosshairGap - kCrosshairLength, centerY, centerX - kCrosshairGap, centerY, color);
	DrawLine(centerX + kCrosshairGap, centerY, centerX + kCrosshairGap + kCrosshairLength, centerY, color);
	//縦線(中心にすきまをあけて2本)
	DrawLine(centerX, centerY - kCrosshairGap - kCrosshairLength, centerX, centerY - kCrosshairGap, color);
	DrawLine(centerX, centerY + kCrosshairGap, centerX, centerY + kCrosshairGap + kCrosshairLength, color);
}

void GameScene::DrawLastHitKessatsuUI()
{
	//ラストヒットイベント用の血殺UI//血:左上、殺:血の少し右下、血の液体:右下寄り
	constexpr int kChiX = 220;//血のX座標
	constexpr int kChiY = 220;//血のY座標

	SetDrawScreen(m_RT1); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	//血:左上
	DrawRectRotaGraph(kChiX, kChiY, 0, 0, kGHX, kGHY, 0.8f, -DX_PI_F / 12, m_gHandle1, TRUE);
	//殺:血の少し右下
	DrawRectRotaGraph(kChiX + 200, kChiY + 170, 0, 0, kGH3X, kGH3Y, 0.5f, 0.0f, m_gHandle3, TRUE);
	//血の液体:右下寄り
	DrawRectRotaGraph(Game::kScreenWidth - Game::kScreenWidth / 15, Game::kScreenHeight - 150, 0, 0, kGH2X, kGH2Y, 1.0f, DX_PI_F / 4, m_gHandle2, TRUE);

	SetDrawScreen(DX_SCREEN_BACK);
	m_cameraManager->ApplyCameraSettings();
	DrawGraph(0, 0, m_RT1, true);
}
void GameScene::Terminate()
{
	//Collisionをすべてクリア
	CollisionManager::GetInstance().Terminate();


	DeleteGraph(m_gHandle1);
	DeleteGraph(m_gHandle2);
	DeleteGraph(m_gHandle3);

	//ResetSceneで何度もGameSceneを作り直せるようになったため、
	//ここで解放しないとDxLibのハンドルがリークし続ける
	DeleteGraph(m_RT1);
	DeleteGraph(m_RT2);
	DeleteShaderConstantBuffer(m_ultShaderHandle);
	DeleteShader(m_enemyPSH);
	DeleteShader(m_enemyVSH);
}
