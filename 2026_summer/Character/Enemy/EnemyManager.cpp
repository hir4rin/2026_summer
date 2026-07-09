#include "EnemyManager.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"
#include "../Character/CharacterBase.h"
#include "../Character/Player/Base/Player.h"
#include "../Game.h"
#include "../DataLoader/DataManager.h"
#include "Collider.h"
#include <cstdlib>
#include <ctime>

namespace
{
	//スポーン座標
	const Vector3 kSpawnWave1 = Vector3(0, 0, 0);
	const Vector3 kSpawnWave2 = Vector3(-6000, 0, 175);
	const Vector3 kSpawnWave3 = Vector3(-12346, 0, 145);
	constexpr float kSpawnWave1Distance = 1000.0f;//wave1のスポーン座標からの距離
}


EnemyManager::EnemyManager(std::weak_ptr<Player> player)
{
	//参照を保存
	m_player = player;
	enemyModelHandle = MV1LoadModel("data/Enemy/sasakiPlayer.mv1");
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

	for (auto& enemy : m_enemies)
	{
		enemy->Update();
	}

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

	//auto removeStart = std::remove_if(m_enemies.begin(),m_enemies.end(),
	//	[](const std::shared_ptr<CharacterBase>& enemy)
	//	{
	//		return enemy->GetIsDead();
	//	});
	//for(auto it = removeStart; it != m_enemies.end(); ++it)
	//{
	//	//Colliderを開放
	//	auto col = (*it)->GetCollider();
	//	if (col)
	//	{
	//		CollisionManager::GetInstance().ReleaseCollider(col);
	//	}
	//}
	//敵を削除
	//m_enemies.erase(removeStart, m_enemies.end());



	std::erase_if(m_enemies, [](const std::shared_ptr<CharacterBase>& enemy)
	{
		return enemy->GetIsDead();
	}
	);
}

void EnemyManager::Draw()
{
	for (auto& enemy : m_enemies)
	{
		enemy->Draw();
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
}
