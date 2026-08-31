#include "EnemyManager.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"
#include "../Character/Enemy/Boss/BossEnemy.h"
#include "../Character/CharacterBase.h"
#include "../Character/Player/Base/Player.h"
#include "../Game.h"
#include "../DataLoader/DataManager.h"
#include "Collider.h"
#include "../WaveAreaCol.h"
#include "../System.h"
#include "../Effect/EffectManager.h"
#include "../Camera/CameraManager.h"
#include "../Camera/CameraState/Stage1FazeCamera.h"
#include "../Camera/CameraState/Stage2FazeCamera.h"
#include <cstdlib>
#include <ctime>

namespace
{
	//スポーン座標
	const Vector3 kSpawnWave1 = Vector3(308, 0, 800);
	const Vector3 kSpawnWave2 = Vector3(205, 0, 3318);
	const Vector3 kSpawnWave3 = Vector3(325, 0, 7183);
	constexpr float kSpawnWave1Distance = 500.0f;//wave1のスポーン座標からの距離

	//wave半径
	constexpr float kWave1Radius = 200.0f;

	//waveのBOXの半径
	const Vector3 kWaveBoxHalfExtent = Vector3(1000, 1250, 200);

	//spawnWaveの座標から壁までの距離(Z軸方向、前後)
	constexpr float kWaveWallDistance = 2000.0f;

	//敵が全滅してからwaveの壁を消すまでの遅延フレーム数
	constexpr int kWaveWallRemoveDelayFrame = 40;
}


EnemyManager::EnemyManager(std::weak_ptr<Player> player)
{
	//参照を保存
	m_player = player;
	enemyModelHandle = MV1DuplicateModel(System::GetInstance().GetHandle(AsyncData::EnemyModel));

	srand(static_cast<unsigned int>(time(nullptr)));//乱数の初期化//これを入れないと、毎回同じ位置に敵が出る
	//5体の敵を出す
	//for (int i = 0; i < 5; i++)
	//{
	//	int handle = MV1DuplicateModel(enemyModelHandle);
	//	ランダムな位置に敵を出す

	//	float PosX = static_cast<float>(rand() % (100 * 2)) - 100;
	//	float PosZ = static_cast<float>(rand() % (100 * 2)) - 100;
	//	生成、初期化
	//	auto enemy = std::make_shared<EnemySwordman>(player, Vector3(PosX, 0.0f, PosZ), handle);
	//	enemy->Init();
	//	m_enemies.push_back(enemy);
	//}
}

EnemyManager::~EnemyManager()
{
	MV1DeleteModel(enemyModelHandle);
}

std::vector<std::shared_ptr<EnemyBase>> EnemyManager::GetEnemies()
{
	auto enemies = m_enemies;//コピーして返す(m_enemies本体やUpdate/Draw側のループには影響させない)
	if (m_boss)
	{
		enemies.push_back(m_boss);//ボスもロックオン/近接ターゲット検索の対象に含める
	}
	return enemies;
}

void EnemyManager::Init()
{
	//ShaderConstantBufferの作成
	m_matCBuffH = CreateShaderConstantBuffer(sizeof(Matrices));
	m_matCBuff = static_cast<Matrices*>(GetBufferShaderConstantBuffer(m_matCBuffH));

	auto& spawnData = DataManager::GetInstance().GetSpawnData();

	for (const auto& data : spawnData)
	{
		SpawnData spawn;
		spawn.name = data[0];
		spawn.pos = Vector3(std::stof(data[1]), std::stof(data[2]), std::stof(data[3]));
		spawn.waveNum = std::stoi(data[4]);
		//waveNumに応じて、ワールド座標を足す
		switch (spawn.waveNum)
		{
		case 1:
			spawn.pos += kSpawnWave1;
			break;
		case 2:
			spawn.pos += kSpawnWave2;
			break;
		case 3:
			spawn.pos += kSpawnWave3;
			break;
		default:
			assert(false && "Invalid wave number!");
			break;
		}
		m_spawnData.push_back(spawn);
	}
	//falseのとき、m_spawnDataが空のとき、assertを出す
	assert(m_spawnData.size() > 0 && "Spawn data is empty!");
	////5体の敵を出す
	//for (int i = 0; i < m_spawnData.size(); i++)
	//{
	//	int handle = MV1DuplicateModel(enemyModelHandle);
	//	//ランダムな位置に敵を出す
	//	//waveNumの場合分けをいつかする

	//	float PosX = m_spawnData[i].pos.x;
	//	float PosY = m_spawnData[i].pos.y;
	//	float PosZ = m_spawnData[i].pos.z;
	//	//生成、初期化
	//	auto enemy = std::make_shared<EnemySwordman>(m_player, Vector3(PosX, PosY, PosZ), handle);
	//	enemy->Init();
	//	m_enemies.push_back(enemy);
	//}
	//CheckSpawnWave();
}

void EnemyManager::Update()
{
	//プレイヤーの現在の位置によって、敵を出す
	CheckSpawnWave();

	//意図的にm_isDeadを1フレーム遅らせることで、死亡時の判定をゲームシーンで取れるようにしている
	//バグったら治す

	//敵が死んでいるかどうかをチェックして、死んでいる敵を消す
	//remove_ifで死んでいる敵を前方に移動//それらが終わった後の開始位置を返す
	for (auto& enemy : m_enemies)
	{
		if (!enemy->GetIsDead())continue;

		auto col = enemy->GetCollider();
		if (col)
		{
			CollisionManager::GetInstance().ReleaseCollider(col);
		}
	}
	//std::remove_ifも考えた
	//削除
	std::erase_if(m_enemies, [](const std::shared_ptr<CharacterBase>& enemy)
	{
		return enemy->GetIsDead();
	}
	);

	//ラストヒットイベント中は、残っている雑魚敵の当たり判定を解放してすべて消去する
	if (System::GetInstance().GetIsLastHitEventPlaying())
	{
		for (auto& enemy : m_enemies)
		{
			auto col = enemy->GetCollider();
			if (col)
			{
				CollisionManager::GetInstance().ReleaseCollider(col);
			}
		}
		m_enemies.clear();
	}

	//すべてのwaveが出ていて、敵がいなくなったら、ゲームクリア//enumは暗黙的にintに変換できないので、キャストする
	//そのwaveの敵がすべて死んでいるかどうかをチェックする
	for(int i = 0; i < static_cast<int>(WaveNum::WaveSize); i++)
	{
		if (m_isSpawnedWave[i])
		{
			if (m_enemies.size() == 0)
			{
				//最後のフェーズはボスを倒したらのため、やらない
				if (i != static_cast<int>(WaveNum::WaveSize) - 1)
				{
					bool wasAlreadyDead = m_isAllEnemiesDead[i];//今フレームで初めてtrueになったかどうかを見るため、上書き前の値を控えておく
					m_isAllEnemiesDead[i] = true;

					//Wave1(index 0)が今フレームで初めてtrueになった瞬間だけ、フェーズ演出カメラに切り替える
					if (i == 0 && !wasAlreadyDead)
					{
						auto cameraManager = m_cameraManager.lock();
						if (cameraManager)
						{
							System::GetInstance().SetIsEventPlaying(true);
							cameraManager->ChangeState(std::make_shared<Stage1FazeCamera>(m_cameraManager));
						}
					}
					//Wave2(index 1)が今フレームで初めてtrueになった瞬間だけ、フェーズ演出カメラに切り替える
					if (i == 1 && !wasAlreadyDead)
					{
						auto cameraManager = m_cameraManager.lock();
						if (cameraManager)
						{
							System::GetInstance().SetIsEventPlaying(true);
							cameraManager->ChangeState(std::make_shared<Stage2FazeCamera>(m_cameraManager));
						}
					}
				}
			}
		}
	}

	//敵が全滅してから数十フレーム経ってから、waveの壁を消す
	if (m_enemies.empty() && !m_waveWalls.empty())
	{
		m_waveWallRemoveTimer++;
		//壁を消す半分の秒数のタイミングで、壁が壊れるエフェクトを出す
		if (m_waveWallRemoveTimer == kWaveWallRemoveDelayFrame / 2)
		{
			for (auto& wall : m_waveWalls)
			{
				//X軸を90度回転させて出す
				EffectManager::GetInstance().Play(AsyncData::WallBreakEffect, wall->GetWorldCenter(), 0.0f, 1.0f, DX_PI_F / 2);
			}
			//エフェクトと同時にSEを鳴らす(最後のフェーズクリア時は鳴らさない)
			if (m_currentWaveNum != static_cast<int>(WaveNum::WaveSize))
			{
				System::GetInstance().GetSoundManager().PlaySE("WallBreakSE");
			}
		}
		if (m_waveWallRemoveTimer >= kWaveWallRemoveDelayFrame)
		{
			for (auto& wall : m_waveWalls)
			{
				CollisionManager::GetInstance().ReleaseCollider(wall);
			}
			m_waveWalls.clear();
			m_waveWallRemoveTimer = 0;
		}
	}

	//最後のフェーズは雑魚全滅ではなく、ボスを倒したかどうかで判定する
	if (m_boss && m_boss->GetIsDead())
	{
		m_isAllEnemiesDead[static_cast<int>(WaveNum::WaveSize) - 1] = true;
	}

	for (auto& enemy : m_enemies)
	{
		enemy->Update();
	}
}

void EnemyManager::Draw()
{
	//カメラ行列のセット
	MATRIX viewMat = GetCameraViewMatrix();
	MATRIX projMat = GetCameraProjectionMatrix();

	for (auto& enemy : m_enemies)
	{
		//world行列をセット
		MATRIX world = MV1GetMatrix(enemy->GetModelHandle());
		m_matCBuff->world = world;
		m_matCBuff->view = viewMat;
		m_matCBuff->projection = projMat;
		UpdateShaderConstantBuffer(m_matCBuffH);
		SetShaderConstantBuffer(m_matCBuffH,DX_SHADERTYPE_VERTEX,4);
		enemy->Draw();
		SetShaderConstantBuffer(-1, DX_SHADERTYPE_VERTEX, 4);
	}
	for (auto& wall : m_waveWalls)
	{
		wall->Draw();
	}
}

void EnemyManager::CheckSpawnWave()
{
	auto player = m_player.lock();
	if (!player)return;
	Vector3 playerPos = player->GetPos();
	playerPos.y = 0.0f;

	for (int i = 0; i < 3; i++)
	{
		Vector3 pos;
		switch (i)
		{
		case 0:
			pos = kSpawnWave1;
			break;
		case 1:
			pos = kSpawnWave2;
			break;
		case 2:
			pos = kSpawnWave3;
			break;
		default:
			assert(false && "Invalid wave number!");
			break;
		}
		float spawnWaveDistance = (playerPos - pos).Magnitude();

		if (spawnWaveDistance < kSpawnWave1Distance && !m_isSpawnedWave[i])
		{
			// Wave 1の敵をスポーンさせる処理
			SpawnEnemies(i + 1);
			m_isSpawnedWave[i] = true;
			//ラストフェーズだったらボスを出現させる
			if (i == static_cast<int>(WaveNum::WaveSize) - 1)
			{
				SpawnBoss();
			}
		}
	}
}

void EnemyManager::SpawnEnemies(int waveNum)
{
	m_currentWaveNum = waveNum;//今アクティブなウェーブ番号を控えておく(壁を消す音を最終フェーズだけ鳴らさないようにするため)
	//waveNumの敵を入れる
	std::vector<SpawnData>dataEnemies;
	for (auto& data : m_spawnData)
	{
		if (data.waveNum == waveNum)
		{
			dataEnemies.push_back(data);
		}
	}
	for (auto& data : dataEnemies)
	{
		SpawnEnemy(data.pos);
	}
	//当たり判定のエリアを生成
	Vector3 spawnPos = {};
	switch (waveNum)
	{
	case 1:
		spawnPos = kSpawnWave1;
		break;
	case 2:
		spawnPos = kSpawnWave2;
		break;
	case 3:
		spawnPos = kSpawnWave3;
		break;
	default:
		assert(false && "Invalid wave number!");
		break;
	}


	//プレイヤーがエリアから出られないようにする壁(前後)//isTriggerはfalseにして押し戻しを効かせる
	auto wallFront = std::make_shared<WaveAreaCol>();
	wallFront->Init(spawnPos + Vector3(0, 0, kWaveWallDistance), Vector3(), kWave1Radius, ColliderType::Box, Tags::WaveArea, true, false);
	wallFront->SetBoxHalfExtents(kWaveBoxHalfExtent);
	wallFront->SetID();
	m_waveWalls.push_back(wallFront);

	auto wallBack = std::make_shared<WaveAreaCol>();
	wallBack->Init(spawnPos - Vector3(0, 0, kWaveWallDistance), Vector3(), kWave1Radius, ColliderType::Box, Tags::WaveArea, true, false);
	wallBack->SetBoxHalfExtents(kWaveBoxHalfExtent);
	wallBack->SetID();
	m_waveWalls.push_back(wallBack);
}

std::shared_ptr<EnemyBase> EnemyManager::SpawnEnemy(Vector3 pos)
{
	int handle = MV1DuplicateModel(enemyModelHandle);
	//生成、初期化
	auto enemy = std::make_shared<EnemySwordman>(m_player, pos, handle);
	enemy->Init();
	enemy->Update();
	m_enemies.push_back(enemy);
	return enemy;
}

void EnemyManager::RestartBossDeadState()
{
	if (!m_boss)return;
	m_boss->RestartDeadState();
}

void EnemyManager::ResetBossToSpawnPos()
{
	if (!m_boss)return;
	m_boss->SetPos(kSpawnWave3);
	m_boss->SetRotY(0.0f);//TODO: 演出に合わせて向きを調整する
}

void EnemyManager::SpawnBoss()
{
	int bossModelHandle = MV1DuplicateModel(System::GetInstance().GetHandle(AsyncData::BossModel));
	m_boss = std::make_shared<BossEnemy>(m_player, Vector3(0, 0, 0), bossModelHandle);
	m_boss->Init();
	m_boss->SetPos(kSpawnWave3 + Vector3());
	//ボスにEnemyManagerの弱参照を渡す(雑魚敵召喚のため)
	m_boss->SetEnemyManager(weak_from_this());
}