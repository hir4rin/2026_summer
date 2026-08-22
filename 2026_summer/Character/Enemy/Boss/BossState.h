#pragma once
#include <memory>

class BossEnemy;

class BossState abstract
{
public:
	BossState(std::weak_ptr<BossEnemy> owner);
	virtual ~BossState() = default;

	virtual void Enter() = 0;//状態に入るときの処理

	virtual void Update() = 0;//状態の更新処理

	virtual void Exit() = 0;//状態から出るときの処理

	virtual void DebugDraw() {};//デバッグ描画//必要な状態でオーバーライドする

protected:
	//状態を持つボスへの弱い参照
	//状態からボスの情報にアクセスするためのもの
	//循環参照を避けるために弱い参照を使う
	std::weak_ptr<BossEnemy> m_owner;

};
