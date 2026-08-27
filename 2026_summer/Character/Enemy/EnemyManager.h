#pragma once
#include "../Math/Vector3.h"
#include <vector>
#include <memory>
#include <string>
class EnemySwordman;
class BossEnemy;
class EnemyBase;
class Camera;
class CameraManager;
class Collider;
class Player;
class WaveAreaCol;

struct SpawnData
{
	std::string name;//名前
	Vector3 pos;//座標
	int waveNum;//ウェーブの番号
};
struct WaveData
{
	int waveNum;//ウェーブの番号
	std::vector<std::shared_ptr<EnemyBase>> enemies;//敵の配列
};
enum class WaveNum : int
{
	Wave1 = 0,
	Wave2 = 1,
	Wave3 = 2,
	WaveSize = 3,
};
//Shader用
struct Matrices
{
	MATRIX world;
	MATRIX view;
	MATRIX projection;
	float various[4] = {};
};

class EnemyManager : public std::enable_shared_from_this<EnemyManager>
{
public:
	EnemyManager(std::weak_ptr<Player> player);
	~EnemyManager();
	void Init();
	void Update();
	void Draw();
	std::shared_ptr<EnemyBase>& GetEnemy(int index) { return m_enemies[index];}
	//雑魚敵一覧+ボス(いれば)をまとめて返す(ロックオン/近接ターゲット検索用)。呼ぶたびにコピーが発生する点に注意
	std::vector<std::shared_ptr<EnemyBase>> GetEnemies();
	std::weak_ptr<Player> GetPlayer() { return m_player; }
	//ボスのゲット(最終フェーズが始まるまではnullptr)
	std::shared_ptr<BossEnemy> GetBoss() { return m_boss; }
	//フェーズ演出でカメラを切り替えるために必要
	void SetCameraManager(std::weak_ptr<CameraManager> cameraManager) { m_cameraManager = cameraManager; }

	//敵をスポーンさせるかチェックし、スポーンさせる
	void CheckSpawnWave();
	void SpawnEnemies(int waveNum);
	//spawnData/waveを経由せず、指定した座標に1体だけ敵を生成する(ボスの召喚攻撃など、任意タイミングでの生成用)
	std::shared_ptr<EnemyBase> SpawnEnemy(Vector3 pos);

	//すべての敵が死んだかどうか
	bool IsGetAllEnemiesDead(int num)const { return m_isAllEnemiesDead[num]; }
	//そのスポーンWaveがスポーンしたかどうか
	bool isSpawnedWave(int num)const { return m_isSpawnedWave[num];}
private:
	//ボスを生成する(CSVのspawnDataは経由せず、直接m_bossに持たせる)
	void SpawnBoss();
private:
	Vector3 m_startPos;//敵の座標

	std::weak_ptr<Player> m_player;//プレイヤーの弱参照
	std::weak_ptr<CameraManager> m_cameraManager;//カメラマネージャーの弱参照(フェーズ演出でのカメラ切り替えに使う)
	std::vector<std::shared_ptr<EnemyBase>> m_enemies;//敵の配列
	std::shared_ptr<BossEnemy> m_boss;//ボス(最終フェーズになるまではnullptr)
	std::vector<std::shared_ptr<WaveAreaCol>> m_waveWalls;//ウェーブのエリア前後の壁(押し戻し用)の配列
	int enemyModelHandle = -1;//敵のモデルのハンドル//EnemySwordManのモデル
	std::vector<SpawnData> m_spawnData;//敵のスポーンデータ
	bool m_isSpawnedWave[3] = {};//ウェーブごとに敵がスポーンしたかどうかのフラグ
	bool m_isAllEnemiesDead[static_cast<int>(WaveNum::WaveSize)] = { false};//全ての敵が死んだかどうかのフラグ

	int m_matCBuffH = -1;
	Matrices* m_matCBuff = nullptr;
};

