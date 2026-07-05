#include "EnemyManager.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"
#include "../Game.h"
#include "../DataLoader/DataManager.h"
#include "Collider.h"
#include <cstdlib>
#include <ctime>

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

	for(const auto& data : spawnData)
	{
		SpawnData spawn;
		spawn.name = data[0];
		spawn.pos = Vector3(std::stof(data[1]), std::stof(data[2]), std::stof(data[3]));
		spawn.waveNum = std::stoi(data[4]);
		m_spawnData.push_back(spawn);
	}
	//falseのとき、m_spawnDataが空のとき、assertを出す
	assert(m_spawnData.size() > 0 && "Spawn data is empty!");
	//5体の敵を出す
	for (int i = 0; i < m_spawnData.size(); i++)
	{
		int handle = MV1DuplicateModel(enemyModelHandle);
		//ランダムな位置に敵を出す
		//waveNumの場合分けをいつかする

		float PosX = m_spawnData[i].pos.x;
		float PosY = m_spawnData[i].pos.y;
		float PosZ = m_spawnData[i].pos.z;
		//生成、初期化
		auto enemy = std::make_shared<EnemySwordman>(m_player, Vector3(PosX, PosY, PosZ), handle);
		enemy->Init();
		m_enemies.push_back(enemy);
	}
}

void EnemyManager::Update()
{
	for (auto& enemy : m_enemies)
	{
		enemy->Update();
	}
	//敵が死んでいるかどうかをチェックして、死んでいる敵を消す//Updateと分けるのが、普通
	/*std::erase_if(m_enemies, [](const std::shared_ptr<Enemy>&enemy)
	{
		return enemy->IsDead();
	}
	);*/
}

void EnemyManager::Draw()
{
	for (auto& enemy : m_enemies)
	{
		enemy->Draw();
	}
}

bool EnemyManager::IsAllEnemyLastHit() const
{
	//for (auto& enemy : m_enemies)
	//{
	//	if (!enemy->GetIsLastHit())//LastHitの時、ほかに敵がいるときは、falseを返す//最後の攻撃で倒した敵がいるときは、trueを返す
	//	{
	//		return false;
	//	}
	//}
	return true;
}
