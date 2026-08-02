#include "EnemyManager.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"
#include "../Character/CharacterBase.h"
#include "../Character/Player/Base/Player.h"
#include "../Game.h"
#include "../DataLoader/DataManager.h"
#include "Collider.h"
#include "../WaveAreaCol.h"
#include "../System.h"
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
	constexpr float kWave1Radius = 1000.0f;
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

	//すべてのwaveが出ていて、敵がいなくなったら、ゲームクリア//enumは暗黙的にintに変換できないので、キャストする
	//そのwaveの敵がすべて死んでいるかどうかをチェックする
	for(int i = 0; i < static_cast<int>(WaveNum::WaveSize); i++)
	{
		if (m_isSpawnedWave[i])
		{
			if (m_enemies.size() == 0)
			{
				m_isAllEnemiesDead[i] = true;
			}
		}
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
		}
	}
}

void EnemyManager::SpawnEnemies(int waveNum)
{
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
		int handle = MV1DuplicateModel(enemyModelHandle);
		//生成、初期化
		auto enemy = std::make_shared<EnemySwordman>(m_player, data.pos, handle);
		enemy->Init();
		enemy->Update();
		m_enemies.push_back(enemy);
	}
	//当たり判定のエリアを生成
	auto waveArea = std::make_shared<WaveAreaCol>();
	switch (waveNum)
	{
	case 0:
		waveArea->ColInit(kSpawnWave1, Vector3(), kWave1Radius, ColliderType::Sphere, Tags::WaveArea, true, true);
		break;
	case 1:
		waveArea->ColInit(kSpawnWave2, Vector3(), kWave1Radius, ColliderType::Sphere, Tags::WaveArea, true, true);
		break;
	case 2:
		waveArea->ColInit(kSpawnWave3, Vector3(), kWave1Radius, ColliderType::Sphere, Tags::WaveArea, true, true);
		break;
	default:
		assert(false && "Invalid wave number!");
		break;
	}
	waveArea->SetID();
	m_waveAreas.push_back(waveArea);
}