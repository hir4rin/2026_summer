#pragma once
#include "CharacterBase.h"

class Player;

class EnemyBase : public CharacterBase
{
public:
	EnemyBase(std::weak_ptr<Player> player);
	virtual ~EnemyBase();
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
protected:
	//ここでやりたいこと
	//・プレイヤーを追いかける
	//・攻撃する
	//・警戒状態//Playerの位置と自分の位置から円を描くように移動
	//・距離を取る//そのまま後ろ側に移動
	std::weak_ptr<Player> m_player;//プレイヤーの弱参照
	/// <summary>
	/// 追いかけるとなったときのplayerの位置を決める(target)
	/// </summary>
	/// <returns>playerの位置を返す</returns>
	Vector3 TargetPlayerPos();
	bool ChasePlayer(Vector3 target,float distance);
	void CautionMove(Vector3 target,float distance);
	void BackMove(Vector3 target, float distance);

};

