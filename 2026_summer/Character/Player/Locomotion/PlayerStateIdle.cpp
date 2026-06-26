#include "PlayerStateIdle.h"
#include "Player.h"
#include "../../../Input.h"
#include "../../../SubWindow/SubWindow.h"

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
	//移動速度を0にする//だんだん遅くするにする予定
	//Runの後だったら専用の切り返しモーションとかやりたい
	player->m_vel = Vector3(0, 0, 0);

}

void PlayerStateIdle::Update()
{
	//weak_ptrからshared_ptrを取得する
	auto player = m_owner.lock();
	if (!player) return;
	auto& input = Input::GetInstance();

	//押し戻しの処理が続かないように消す
	player->m_vel = Vector3(0, 0, 0);

	//移動状態に遷移する
	if (input.IsLeftStickInput())
	{
		player->ChangeState(std::make_shared<PlayerStateMove>(m_owner));
		return;
	}

	//攻撃状態に遷移する
	if (input.IsTriggered("X"))//弱攻撃
	{
		player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner,AttackType::lightAttack));
		return;
	}
	if (input.IsTriggered("Y"))//強攻撃
	{
		player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner,AttackType::heavyAttack));
		return;
	}

	if (input.IsTriggered("LB"))
	{
		player->ChangeState(std::make_shared<PlayerStateParry>(m_owner));
		return;
	}

	//回避状態に遷移する
	if (input.IsTriggered("B") && player->IsAvoidable())
	{
		player->ChangeState(std::make_shared<PlayerStateAvoid>(m_owner));
		return;
	}
	//ジャンプ
	if (input.IsTriggered("A") )
	{
		player->ChangeState(std::make_shared<PlayerStateJump>(m_owner));
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
	SubWindow::AddText("PlayerState:Idle");
}
