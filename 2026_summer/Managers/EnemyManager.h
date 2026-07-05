#pragma once
#include "../Math/Vector3.h"
#include <vector>
#include <memory>
#include <string>
class EnemySwordman;
class EnemyBase;
class Camera;
class Collider;
class Player;

struct SpawnData
{
	std::string name;//名前
	Vector3 pos;//座標
	int waveNum;//ウェーブの番号
};

class EnemyManager
{
public:
	EnemyManager(std::weak_ptr<Player> player);
	~EnemyManager();
	void Init();
	void Update();
	void Draw();
	std::shared_ptr<EnemyBase>& GetEnemy(int index) { return m_enemies[index];}
	std::vector<std::shared_ptr<EnemyBase>>& GetEnemies(){ return m_enemies;}

	bool IsAllEnemyLastHit()const;//全ての敵が最後の攻撃で倒されたかどうかを返す//カメラ演出の切り替えに使う予定

private:
	Vector3 m_startPos;//敵の座標

	std::weak_ptr<Player> m_player;//プレイヤーの弱参照
	std::vector<std::shared_ptr<EnemyBase>> m_enemies;//敵の配列
	int enemyModelHandle = -1;//敵のモデルのハンドル//EnemySwordManのモデル
	std::vector<SpawnData> m_spawnData;//敵のスポーンデータ

};

