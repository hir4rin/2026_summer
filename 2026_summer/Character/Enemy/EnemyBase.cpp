#include "EnemyBase.h"
#include "Player.h"
#include "../../Game.h"

EnemyBase::EnemyBase(std::weak_ptr<Player> player)
	: m_player(player)
{
}

EnemyBase::~EnemyBase()
{
}

Vector3 EnemyBase::TargetPlayerPos()
{
	if (auto player = m_player.lock())
	{
		return player->GetPos();
	}

	return Vector3();
}

bool EnemyBase::ChasePlayer(Vector3 target,float distance)
{
	//プレイヤーの位置に向かって移動する//Y軸は移動しない
	target.y = 0.0f;
	//プレイヤーの手前側が目的地になるように移動//内積は今回はしない
	Vector3 toPlayer = target - m_pos;//プレイヤーへのベクトル

	//playerとの距離を図り、手前側かつ、指定距離まで来たら移動を止める
	if (toPlayer.Magnitude() <= distance)
	{
		//目的地に到達した
		m_vel = Vector3(0, 0, 0);
		return true;
	}
	//速度を指定
	m_vel = toPlayer.Normalize() * Game::kEnemyMoveSpeed;

	return false;

}

void EnemyBase::CautionMove(Vector3 target,float distance)
{
	//プレイヤーの位置と自分の位置から円を描くように移動する//Y軸は移動しない
	//playerからEnemyへのベクトルの接線方向に移動
	Vector3 toEnemy = m_pos - target;
	toEnemy.y = 0.0f;
	//接線の求め方はベクトルを90度回転させるので成分を入れ替えて、xを符号反転　
	Vector3 tangentLine = Vector3(-toEnemy.z, 0.0f, toEnemy.x).Normalize();
	m_vel = tangentLine * Game::kEnemyMoveSpeed;
	//半径を維持するために後ろ側にもベクトルを加える
	float dist = toEnemy.Magnitude();
	if (dist < distance)
	{
		Vector3 backVec = toEnemy.Normalize() * (distance - dist) * 0.2f;//半径を維持するためのベクトル//
		m_vel += backVec;
	}
}

void EnemyBase::BackMove(Vector3 target, float distance)
{
	//ToEnemyの方向に移動する
	Vector3 toEnemy = m_pos - target;
	toEnemy.y = 0.0f;
	m_vel = toEnemy.Normalize() * Game::kEnemyBackSpeed;
}
