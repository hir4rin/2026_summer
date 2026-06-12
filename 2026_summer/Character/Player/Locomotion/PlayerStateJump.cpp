#include "PlayerStateJump.h"
#include "PlayerStateJump.h"
#include "Player.h"
#include "../../../Game.h"
#include "../../../Input.h"


namespace
{
	constexpr float kJumpInitVel = 16.0f;//ジャンプの初速//この数値を変えることで、ジャンプの高さを調整できる
}


PlayerStateJump::PlayerStateJump(std::weak_ptr<Player> player) : PlayerState(player)
{
	//playerが既に破棄されていたら早期リターンする
	if (m_owner.expired())return;
}

PlayerStateJump::~PlayerStateJump()
{
}

void PlayerStateJump::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;
	//animationの初期化
	
	//上昇速度を与える
	player->m_vel.y = kJumpInitVel;//ジャンプの初速//この数値を変えることで、ジャンプの高さを調整できる
	//ジャンプ状態
	player->m_isGround = false;//地面にいない状態にする
	//ジャンプ開始時の移動速度を保存する
	m_baseVel = player->m_vel;
	m_baseVel.y = 0.0f;//y成分は移動に関係ないので、0にする
}

void PlayerStateJump::Update()
{
	//weak_ptrからshared_ptrを取得する
	auto player = m_owner.lock();
	if (!player) return;
	auto& input = Input::GetInstance();

	player->m_vel += Vector3(0, -Game::kGravity, 0);//重力の処理

	//攻撃状態に遷移する
	if (input.IsTriggered("X"))//弱攻撃
	{
		player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner, AttackType::lightAttack));
		return;
	}
	if (input.IsTriggered("Y"))//強攻撃
	{
		player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner, AttackType::heavyAttack));
		return;
	}

	if (player->m_vel.y <= 0.0f)//下降中に移行
	{
		player->ChangeState(std::make_shared<PlayerStateFall>(m_owner));//Fall状態に遷移する
		return;
	}
	//移動処理
	Move(input);
}

void PlayerStateJump::Exit()
{
}

void PlayerStateJump::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Jump");
}

void PlayerStateJump::Move(Input& input)
{
	//weak_ptrからshared_ptrを取得する
	auto player = m_owner.lock();
	if (!player) return;
	//Playerの移動方向を決める
	HandlerInput();
	//移動入力をとる
	Vector3 dimention = Vector3(0, 0, 0);
	if (input.IsPressed("Up"))
	{
		dimention += player->forward;
	}
	if (input.IsPressed("Down"))
	{
		dimention += player->down;
	}
	if (input.IsPressed("Right"))
	{
		dimention += player->right;
	}
	if (input.IsPressed("Left"))
	{
		dimention += player->left;
	}
	dimention = dimention.Normalize() * Game::kMoveSpeed * 0.5f;//移動速度は通常の0.5倍
	Vector3 velY = Vector3(0, player->m_vel.y, 0);//y成分だけを取り出す
	player->m_vel = m_baseVel + dimention + velY;
	//速度制限
	ClampSpeed();

	//移動している間は目標のベクトルを更新する
	player->m_targetVec = player->m_vel.Normalize();
}
