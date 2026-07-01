#include "EnemyManager.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"
#include "../Game.h"
#include "Collider.h"
#include <cstdlib>
#include <ctime>

EnemyManager::EnemyManager(std::weak_ptr<Player> player)
{
	//参照を保存
	m_player = player;
	enemyModelHandle = MV1LoadModel("data/Enemy/swordman.mv1");
	srand(static_cast<unsigned int>(time(nullptr)));//乱数の初期化//これを入れないと、毎回同じ位置に敵が出る
	//5体の敵を出す
	for (int i = 0; i < 5; i++)
	{
		int handle = MV1DuplicateModel(enemyModelHandle);
		//ランダムな位置に敵を出す

		
		float PosX = static_cast<float>(rand() % (100 * 2)) - 100;//-XからXのランダムな数を生成する
		float PosZ = static_cast<float>(rand() % (100 * 2)) - 100;//-600から600のランダムな数を生成する
		//生成、初期化
		auto enemy = std::make_shared<EnemySwordman>(player,Vector3(PosX, 0.0f, PosZ), handle);
		enemy->Init();
		m_enemies.push_back(enemy);
	}
}

EnemyManager::~EnemyManager()
{
	MV1DeleteModel(enemyModelHandle);
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
	//return true;
}
