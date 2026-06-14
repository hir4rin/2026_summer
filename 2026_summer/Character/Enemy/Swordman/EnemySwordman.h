#pragma once
#include "../EnemyBase.h"
class EnemySwordman : public EnemyBase
{
public:
	EnemySwordman(std::weak_ptr<Player> player);
	virtual ~EnemySwordman();
	void Init() override;
	void Update() override;
	void Draw() override;

	void Attack();

	void OnCollision(Collider& other) override;
private:
	void ChangeState(EnemyState newState) override;
	//一旦表示をさせる、次にEnemyBaseの関数を呼び出して確認する
};

