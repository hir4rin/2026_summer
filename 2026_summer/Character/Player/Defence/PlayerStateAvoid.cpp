#include "PlayerStateAvoid.h"
#include "Player.h"
#include "../../../Input.h"
#include "../../../Camera/Camera.h"
#include "../../../System.h"
#include "../../../Input.h"

namespace 
{
	constexpr float kAvoidCoolTime = 12.0f;//回避のクールタイム
	constexpr float kAvoidFrame = 20.0f;//回避のフレーム数//回避の時間を管理するための定数
	constexpr float kAvoidSpeed = 18.0f;//回避の速度
}

PlayerStateAvoid::PlayerStateAvoid(std::weak_ptr<Player> player):
		PlayerState(player)
{
	//playerが既に破棄されていたら早期リターンする
	if (m_owner.expired())return;
}

PlayerStateAvoid::~PlayerStateAvoid()
{
}

void PlayerStateAvoid::Enter()
{
	auto player = m_owner.lock();
	if (!player) return;

	//回避方向を決定//回避アニメーションの初期化
	DetermineAvoidDirection();
	//ルートモーションを有効にする
	//player->m_anim.SetRootMotionEnable(true, 1);
	//回避の情報を初期化
	player->m_avoidInfo.avoidCount = 0.0f;
	player->m_avoidInfo.avoidCoolTimeCount = kAvoidCoolTime;//回避のクールタイムを12フレームに設定

}

void PlayerStateAvoid::Update()
{
	auto player = m_owner.lock();
	if (!player) return;
	auto& input = Input::GetInstance();

	player->m_avoidInfo.avoidCount += 1.0f * System::GetInstance().GetTimeScale();
	if(player->m_avoidInfo.avoidCount >= kAvoidFrame)
	{
		if (input.IsLeftStickInput())
		{
			//入力があればRun状態に遷移する
			player->ChangeState(std::make_shared<PlayerStateRun>(m_owner));
			return;
		}
		else
		{
			//回避のフレーム数を超えたら、Idle状態に遷移する
			player->ChangeState(std::make_shared<PlayerStateIdle>(m_owner));
			return;
		}
		
	}

	//アニメーションの更新
	player->m_anim.Update();
	//速度の指定
	//player->m_vel = player->m_avoidInfo.BaseVel.Normalize() * kAvoidSpeed;//回避の速度を0.3fに設定//回避の方向を正規化して、速度を指定する
	//Vector3 rootDelta = player->m_anim.GetRootMotionDelta();//ルートモーションの移動量を取得する
	//player->m_vel = rootDelta * 1.0f;//ルートモーションの移動量に回避の速度を掛けることで、回避の移動量を指定する

}

void PlayerStateAvoid::Exit()
{
	auto player = m_owner.lock();
	if (!player) return;

	//ルートモーションを無効にする
	player->m_anim.SetRootMotionEnable(false);
	//回避の情報をリセットする
	player->m_avoidInfo.BaseVel = Vector3();
	player->m_avoidInfo.avoidCount = 0.0f;
	player->m_avoidInfo.avoidCoolTimeCount = kAvoidCoolTime;
}

void PlayerStateAvoid::DebugDraw()
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "PlayerState:Avoid");
}

void PlayerStateAvoid::DetermineAvoidDirection()
{
	auto player = m_owner.lock();
	if (!player) return;

	auto& input = Input::GetInstance();
	auto& camera = player->m_camera;

	//回避の方向を決める//カメラの向きと入力から、回避の方向を決める
	if(input.IsPressed("Up"))
	{
		player->m_avoidInfo.BaseVel += player->forward;
	}
	if(input.IsPressed("Down"))
	{
		player->m_avoidInfo.BaseVel += player->down;
	}
	if(input.IsPressed("Left"))
	{
		player->m_avoidInfo.BaseVel += player->left;
	}
	if(input.IsPressed("Right"))
	{
		player->m_avoidInfo.BaseVel += player->right;
	}
	//入力がないときは、playerの後ろに回避する
	if(player->m_avoidInfo.BaseVel.Magnitude() == 0.0f)
	{
		player->m_avoidInfo.BaseVel = player->m_targetVec * -1.0f;//カメラの向きの逆方向に回避する
		player->m_anim.ChangeAnim(player->GetAnimName("DodgeBackward"), false,1.0f);
		player->m_avoidInfo.isAvoidBack = true;
	}
	else
	{
		player->m_avoidInfo.isAvoidBack = false;
		player->m_anim.ChangeAnim(player->GetAnimName("DodgeForward"), false,1.0f);
	}

}

