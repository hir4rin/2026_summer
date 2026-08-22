#pragma once
#include "../EnemyBase.h"
#include "../../../Math/Vector3.h"
#include "BossStateIdle.h"
#include "BossStateCaution.h"
#include "BossStateChase.h"
#include "BossStateAttack.h"
#include "BossStateBack.h"
#include "BossStateHit.h"
#include "BossStateAirStay.h"
#include "BossStateFall.h"
#include "BossStateDead.h"
#include "BossStateKnockDown.h"

class AttackCol;
class BossEnemy : public EnemyBase
{
public:
	BossEnemy(std::weak_ptr<Player> player,Vector3 pos,int modelHandle);
	virtual ~BossEnemy();
	void Init() override;
	void Update() override;
	void Draw() override;
	void Attack();

	void OnCollision(Collider& other) override;
	void OnDamage(Collider& other, AttackData& data) override;
	void Terminate();
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
private:
	std::shared_ptr<BossState> m_currentState;//ボスの状態//攻撃中、追跡中など//状態遷移の管理をするためのもの

	std::shared_ptr<AttackCol>  m_attackCol;
	float m_cautionUpdateTimer = 0.0f;
	int m_hitEfHandle = -1;//ヒットエフェクトのハンドル
	int m_hitEfPlayingHandle = -1;//再生中のヒットエフェクトのハンドル

	//BossState
	friend class BossState;//BossStateクラスから、BossEnemyクラスのprivateメンバにアクセスできるようにする
	friend class BossStateIdle;
	friend class BossStateCaution;
	friend class BossStateChase;
	friend class BossStateAttack;
	friend class BossStateBack;
	friend class BossStateHit;
	friend class BossStateAirStay;
	friend class BossStateFall;
	friend class BossStateDead;
	friend class BossStateKnockDown;

};
