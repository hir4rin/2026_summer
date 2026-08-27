#pragma once
#include "../EnemyBase.h"
#include "../../../Math/Vector3.h"
#include "BossStateIdle.h"
#include "BossStateCaution.h"
#include "BossStateChase.h"
#include "BossStateAttack.h"
#include "BossStateBack.h"
#include "BossStateHit.h"
#include "BossStateDead.h"
#include <vector>

class AttackCol;
class EnemyManager;
class BossEnemy : public EnemyBase
{
public:
	enum class AttackType
	{
		None,
		Melee,
		Langed,
		Summon
	};

public:
	BossEnemy(std::weak_ptr<Player> player,Vector3 pos,int modelHandle);
	virtual ~BossEnemy();
	void Init() override;
	void Update() override;
	void Draw() override;
	void Attack();
	//出現位置
	void SetPos(Vector3 pos) { m_pos = pos; }

	void OnCollision(Collider& other) override;
	void OnDamage(Collider& other, AttackData& data) override;
	void Terminate();

	void SetEnemyManager(std::weak_ptr<EnemyManager> enemyManager) { m_enemyManager = enemyManager; }//EnemyManagerのセット
private:
	void ChangeState(EnemyState newState) override;//EnemyBaseのインターフェース用//BossStateへの遷移に変換して呼ぶ
	void ChangeState(std::shared_ptr<BossState> newState);//状態遷移の関数//現在の状態のExitを呼び、新しい状態に切り替えてEnterを呼ぶ

	//一旦表示をさせる、次にEnemyBaseの関数を呼び出して確認する
	std::shared_ptr<BossEnemy> GetSharedPtr() { return std::dynamic_pointer_cast<BossEnemy>(shared_from_this()); }
	std::shared_ptr<BossEnemy> GetWeakPtr() { return GetSharedPtr(); }
	/// <summary>
	/// intervalフレーム事にtrueを返す関数
	/// </summary>
	/// <param name="timer"></param>
	/// <param name="interval"></param>
	/// <returns></returns>
	bool TickInterval(float& timer, float interval);//タイマーのインターバルをチェックする//trueならインターバルが経過した
	void ApplyPos()override;//座標の適用//
private:
	std::shared_ptr<BossState> m_currentState;//ボスの状態//攻撃中、追跡中など//状態遷移の管理をするためのもの

	std::shared_ptr<AttackCol>  m_attackCol;
	float m_cautionUpdateTimer = 0.0f;
	int m_hitEfHandle = -1;//ヒットエフェクトのハンドル
	int m_hitEfPlayingHandle = -1;//再生中のヒットエフェクトのハンドル

	float m_stunStack = 0.0f;//100たまったらスタンステートに移行

	AttackType m_attackType = AttackType::None;
	std::vector<std::shared_ptr<AttackCol>> m_attackCols;

	std::weak_ptr<EnemyManager> m_enemyManager;//EnemyManagerの弱参照//雑魚敵の召喚に使う
	

	//BossState
	friend class BossState;//BossStateクラスから、BossEnemyクラスのprivateメンバにアクセスできるようにする
	friend class BossStateIdle;
	friend class BossStateCaution;
	friend class BossStateChase;
	friend class BossStateAttack;
	friend class BossStateBack;
	friend class BossStateHit;
	friend class BossStateDead;

};
