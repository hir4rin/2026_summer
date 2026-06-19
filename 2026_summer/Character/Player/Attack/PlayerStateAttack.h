#pragma once
#include "PlayerState.h"
#include "PlayerEnums.h"
#include <memory>
class Input;
class AttackCol;

class PlayerStateAttack : public PlayerState
{
public:
	PlayerStateAttack(std::weak_ptr<Player> player,AttackType type);
	virtual ~PlayerStateAttack();
	void Enter() override;
	void Update() override;
	void Exit() override;
	void DebugDraw()override;
private:
	void AttackMoveMent();//攻撃中の移動処理
	void DetermineAttackDirection();//攻撃の方向を決める関数
	void AttackInputCheck();//攻撃入力をチェックする関数
	void StartCombo(int comboIndex);//コンボを開始する関数//comboIndexは、次のコンボの段数
	void AttackFinishProcess();//攻撃が終了したときの処理//コンボの段数を初期化するなど
private:
	AttackType m_attackType;//攻撃のタイプ//弱攻撃か強攻撃か
	int m_nextComboIndex = -1;//次のコンボの段数
	bool m_isComboInputReserved = false;//コンボ入力を受け付けたかどうかのフラグ
	std::shared_ptr<AttackCol> m_attackCol;//攻撃の当たり判定
};

