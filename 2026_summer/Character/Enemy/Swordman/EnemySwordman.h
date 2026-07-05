#pragma once
#include "../EnemyBase.h"
#include "../../../Math/Vector3.h"

class AttackCol;
class EnemySwordman : public EnemyBase, public std::enable_shared_from_this<EnemySwordman>
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
private:
	void ChangeState(EnemyState newState) override;
	//一旦表示をさせる、次にEnemyBaseの関数を呼び出して確認する

private:
	std::shared_ptr<AttackCol>  m_attackCol;
	
};

