#pragma once
#include "../EnemyBase.h"
#include "../../../Math/Vector3.h"

class AttackCol;
class EnemySwordman : public EnemyBase
{
public:
	EnemySwordman(std::weak_ptr<Player> player,Vector3 pos,int modelHandle);
	virtual ~EnemySwordman();
	void Init() override;
	void Update() override;
	void Draw() override;
	std::string  GetHitString();
	void Attack();

	void OnCollision(Collider& other) override;
	void OnDamage(Collider& other, AttackData& data) override;
	void Terminate();
private:
	void ChangeState(EnemyState newState) override;
	//一旦表示をさせる、次にEnemyBaseの関数を呼び出して確認する
	std::shared_ptr<EnemySwordman> GetSharedPtr() { return std::dynamic_pointer_cast<EnemySwordman>(shared_from_this()); }
	std::shared_ptr<EnemySwordman> GetWeakPtr() { return GetSharedPtr(); }
	/// <summary>
	/// intervalフレーム事にtrueを返す関数
	/// </summary>
	/// <param name="timer"></param>
	/// <param name="interval"></param>
	/// <returns></returns>
	bool TickInterval(float& timer, float interval);//タイマーのインターバルをチェックする//trueならインターバルが経過した
private:
	std::shared_ptr<AttackCol>  m_attackCol;
	float m_cautionUpdateTimer = 0.0f;
	int m_hitEfHandle = -1;//ヒットエフェクトのハンドル
	int m_hitEfPlayingHandle = -1;//再生中のヒットエフェクトのハンドル
	
};

