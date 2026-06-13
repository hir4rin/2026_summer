#include "PlayerStateParry.h"
#include "Player.h"
#include "../../../Input.h"

PlayerStateParry::PlayerStateParry(std::weak_ptr<Player> player):
	PlayerState(player)
{
	//playerが既に破棄されていたら早期リターンする
	if (m_owner.expired())return;
}

PlayerStateParry::~PlayerStateParry()
{
}

void PlayerStateParry::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;
	//パリィのアニメーションの初期化
	player->m_anim.ChangeAnim(player->GetAnimName("AttackPose"), false);
	//速度の初期化
	player->m_vel = Vector3(0, 0, 0);//パリィ中は動かないようにする

}

void PlayerStateParry::Update()
{
	auto player = m_owner.lock();
	if (!player) return;

	//ボタンを離したらParryをやめる
	auto& input = Input::GetInstance();
	if (!input.IsPressed("LB"))
	{
		//LBボタンが離されたら、Idle状態に遷移する


		player->ChangeState(std::make_shared<PlayerStateIdle>(m_owner));
		return;
	}

	//アニメーションの更新
	player->m_anim.Update();


}

void PlayerStateParry::Exit()
{
}

void PlayerStateParry::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Parry");
}
