#include "PlayerStateIdle.h"
#include "Player.h"
#include "../../Input.h"

PlayerStateIdle::PlayerStateIdle(std::weak_ptr<Player> player) :
	PlayerState(player)
{
	//playerが既に破棄されていたら早期リターンする
	if (m_owner.expired())return;
}

PlayerStateIdle::~PlayerStateIdle()
{
}
void PlayerStateIdle::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;
	//animationの初期化
	player->m_anim.ChangeAnim(player->GetAnimName("Idle"), true);
	//移動速度を0にする
	player->m_vel = Vector3(0, 0, 0);

}

void PlayerStateIdle::Update()
{
	//weak_ptrからshared_ptrを取得する
	auto player = m_owner.lock();
	if (!player) return;
	auto& input = Input::GetInstance();

	if (input.IsLeftStickInput())
	{
		//移動状態に遷移する
		player->ChangeState(std::make_shared<PlayerStateMove>(m_owner));
		return;
	}


	//アニメーションの更新
	player->m_anim.Update();
}

void PlayerStateIdle::Exit()
{
}

void PlayerStateIdle::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Idle");
}
