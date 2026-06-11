#include "PlayerStateJump.h"
#include "PlayerStateJump.h"
#include "Player.h"
#include "../../../Input.h"

namespace
{
	constexpr float kJumpInitVel = 12.0f;//ジャンプの初速//この数値を変えることで、ジャンプの高さを調整できる
	constexpr float kGravity = 0.6f;//重力の加速度//この数値を変えることで、ジャンプの高さや落ちる速さを調整できる
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
}

void PlayerStateJump::Update()
{
	//weak_ptrからshared_ptrを取得する
	auto player = m_owner.lock();
	if (!player) return;
	auto& input = Input::GetInstance();

	player->m_vel += Vector3(0, -kGravity, 0);//重力の処理//だんだん落ちていくようにする

	if (player->m_pos.y <= 0.0f)//地面に着いたら
	{
		player->m_pos.y = 0.0f;//地面に埋まらないようにする
		player->ChangeState(std::make_shared<PlayerStateIdle>(m_owner));//Idle状態に遷移する
		return;
	}
}

void PlayerStateJump::Exit()
{
}

void PlayerStateJump::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Jump");
}
