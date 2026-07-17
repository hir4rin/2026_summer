#include "PlayerStateRun.h"
#include "PlayerStateRun.h"
#include "Player.h"
#include "../../../Input.h"
#include "../../../Camera/Camera.h"


namespace
{
	const float kRunSpeed = 10.0f;//移動速度
}

PlayerStateRun::PlayerStateRun(std::weak_ptr<Player> player):
	PlayerState(player)
{
	//playerが既に破棄されていたら早期リターンする
	if (m_owner.expired())return;
}

PlayerStateRun::~PlayerStateRun()
{
}

void PlayerStateRun::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;
	//animationの初期化
	player->m_anim.ChangeAnimWithModelHandle(player->m_modelHandle, player->GetAnimName("Run"), true,1.0f);
}

void PlayerStateRun::Update()
{
	//weak_ptrからshared_ptrを取得する
	auto player = m_owner.lock();
	if (!player) return;
	auto& input = Input::GetInstance();

	//run中は上下差がないので、毎フレームすべて初期化
	player->m_vel = Vector3(0, 0, 0);
	//鴉状態の更新
	if (input.IsPressed("LB"))
	{
		player->m_isRaven = true;
	}
	else
	{
		player->m_isRaven = false;
	}

	if (!input.IsLeftStickInput())
	{
		//移動状態に遷移する
		player->ChangeState(std::make_shared<PlayerStateIdle>(m_owner));
		return;
	}
	//スキル攻撃
	if (input.IsPressed("LB") && input.IsTriggered("X"))
	{
		if (player->CanSkillAttack())
		{
			player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner, AttackType::SkillAttack));
			return;
		}
	}
	//必殺技
	if (input.IsPressed("LB") && input.IsTriggered("Y"))
	{
		if (player->CanUltAttack())
		{
			player->ChangeState(std::make_shared<PlayerStateUlt>(m_owner));
			return;
		}
	}

	//攻撃状態に遷移する
	if (input.IsTriggered("X"))//突進攻撃
	{
		player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner,AttackType::lightAttack));
		return;
	}
	if (input.IsTriggered("Y"))//強攻撃
	{
		player->ChangeState(std::make_shared<PlayerStateAttack>(m_owner, AttackType::heavyAttack));
		return;
	}

	//回避状態に遷移する
	if (input.IsTriggered("B") && player->IsAvoidable())
	{
		player->ChangeState(std::make_shared<PlayerStateAvoid>(m_owner));
		return;
	}
	//ジャンプ
	if (input.IsTriggered("A"))
	{
		player->ChangeState(std::make_shared<PlayerStateJump>(m_owner));
		return;
	}


	//アニメーションの更新
	player->m_anim.Update();
	//移動処理
	Move(input);
}

void PlayerStateRun::Exit()
{
}

void PlayerStateRun::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Run");
}

void PlayerStateRun::Move(Input& input)
{
	//weak_ptrからshared_ptrを取得する
	auto player = m_owner.lock();
	if (!player) return;
	//移動方向の初期化//毎フレーム、カメラからプレイヤーへのベクトルを求めて、移動方向を決める
	{
		//前後移動を最初に決める
		Vector3 CameraToPlayer = player->m_pos - (player->m_camera->GetCameraPos());//カメラからプレイヤーへのベクトル
		//初期化
		Vector3 VelSize = CameraToPlayer.Normalize();//カメラからプレイヤーへのベクトルを正規化して、移動速度を5にする
		VelSize.y = 0.0f;//y成分は移動に関係ないので、0にする

		player->forward = VelSize.Normalize();
		player->down = player->forward * -1.0f;
		player->left = player->forward.Cross(Vector3(0, 1, 0)).Normalize();
		player->right = player->left * -1.0f;
	}
	//移動入力をとる
	if (input.IsPressed("Up"))
	{
		player->m_vel += player->forward;
	}
	if (input.IsPressed("Down"))
	{
		player->m_vel += player->down;
	}
	if (input.IsPressed("Right"))
	{
		player->m_vel += player->right;
	}
	if (input.IsPressed("Left"))
	{
		player->m_vel += player->left;
	}

	//移動している間は目標のベクトルを更新する
	player->m_targetVec = player->m_vel.Normalize();
	//初期化
	player->m_vel = player->m_vel.Normalize() * kRunSpeed;//移動速度を5にする
}
