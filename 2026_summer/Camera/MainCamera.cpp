#include "MainCamera.h"
#include "CameraManager.h"
#include "Player.h"
#include "../Character/Enemy/EnemyBase.h"
#include "../SubWindow/SubWindow.h"
#include <algorithm>

namespace
{
	constexpr float kDeadZoneRadius = 0.5f;//ラープのデッドゾーン
	constexpr float kFullLerpRadius = 2.0f;//この距離以上で通常のラープ

	constexpr float kPosLerpScale = 0.055f;//この値を距離にかけてクランプする//大きいとラープ最大値に行きやすく、小さいとラープ最小値に行きやすい
	constexpr float kPosLerpMin = 0.05f;//ラープの最小値
	constexpr float kPosLerpMax = 0.08f;//ラープの最大値

}

MainCamera::MainCamera()
{
}

MainCamera::~MainCamera()
{
}

void MainCamera::Init()
{
}

void MainCamera::Draw()
{
	//m_pos1とm_targetを描画
	std::string posText = "CameraPos:(" + std::to_string(m_pos.x) + "," + std::to_string(m_pos.y) + "," + std::to_string(m_pos.z) + ")";
	std::string targetText = "CameraTarget:(" + std::to_string(m_target.x) + "," + std::to_string(m_target.y) + "," + std::to_string(m_target.z) + ")";
	SubWindow::AddText(posText);
	SubWindow::AddText(targetText);
}

void MainCamera::Update(Vector3 pos, Vector3 pos2)
{
	//ラープだったらラープ、SlerpだったらSlerpする

}

void MainCamera::Update(const CameraData& data)
{
	//データを保存
	//m_cameraData = data;
	auto player = m_cameraContext->m_player.lock();
	if (!player)return;
	auto enemy = m_cameraContext->m_targetEnemy.lock();

	//ターゲットのらーぷをする
	if (m_isLerp[0])
	{
		float dist = (data.target - m_target).Magnitude();
		if (dist > kDeadZoneRadius)
		{
			//デッドゾーンからフルラープまでの割合(0~1)で割合を変える
			float t = std::clamp((dist - kDeadZoneRadius) / (kFullLerpRadius - kDeadZoneRadius), 0.0f, 1.0f);
			m_target = Vector3::Lerp(m_pos, data.target, 0.3f * t);

		}

		//m_target = Vector3::Lerp(m_pos,data.target,0.3f);
		//ターゲットとの値が近くなったら、ラープをやめる
		/*if ((m_target - data.target).Magnitude() < 0.15f)
		{
			m_isLerp[0] = false;
		}*/
	}
	//Slerpだが、注視点はLerpする
	else if (m_isSlerp[0])
	{
		float dist = (data.target - m_target).Magnitude();
		if (dist > kDeadZoneRadius)
		{
			//デッドゾーンからフルラープまでの割合(0~1)で割合を変える
			float t = std::clamp((dist - kDeadZoneRadius) / (kFullLerpRadius - kDeadZoneRadius), 0.0f, 1.0f);
			m_target = Vector3::Lerp(m_pos, data.target, 0.3f * t);

		}
		//m_target = Vector3::Lerp(m_pos, data.target, 0.3f);
		////ターゲットとの値が近くなったら、ラープをやめる
		//if ((m_target - data.target).Magnitude() < 0.15f)
		//{
		//	m_isSlerp[0] = false;
		//}
	}
	else
	{
		m_target = data.target;
	}
	//----------------------------------------------------------------------------------------------------------
	//ラープだったらラープ、SlerpだったらSlerpする
	//まずはラープなしで動かす
	if (m_isLerp[1])
	{
		//距離に応じてラープの割合を変える
		float posDist = (data.pos - m_pos).Magnitude();
		float lerpT = std::clamp(posDist * kPosLerpScale, kPosLerpMin, kPosLerpMax);
		////現在座標から目標までの保管をしながらm_posを計算する
		m_pos = Vector3::Lerp(m_pos, data.pos, lerpT);
		
		//m_target = data.target;
		//ターゲットとの値が近くなったら、ラープをやめる
	/*	if ((m_pos - data.pos).Magnitude() < 0.15f)
		{
			m_isLerp[1] = false;
		}*/

	}
	else if (m_isSlerp[1])
	{
		//敵がいるなら、m_rotateAxisを更新
		//m_targetPosも更新
		if (enemy)m_rotateAxis = enemy->GetPos();

		//方向はSlerpで、距離はLerpでやる
		//m_distance;
		float distanceTarget = (data.pos - m_rotateAxis).Magnitude();

		Vector3 dirPrev = (m_pos - m_rotateAxis).Normalize();
		Vector3 dirTarget = (data.pos - m_rotateAxis).Normalize();

		//距離に応じてラープの割合を変える
		float posDist = (data.pos - m_pos).Magnitude();
		float lerpT = std::clamp(posDist * kPosLerpScale, kPosLerpMin, kPosLerpMax);

		m_distance = std::lerp(m_distance, distanceTarget, lerpT);
		//現在座標から目標までの保管をしながらm_posを計算する
		m_pos = Vector3::Slerp(dirPrev, dirTarget, lerpT) * m_distance + m_rotateAxis;
		//m_target = data.target;
		//ターゲットとの値が近くなったら、スラープをやめる
		/*if ((m_pos - data.pos).Magnitude() < 0.15f)
		{
			m_isSlerp[1] = false;
		}*/
	}
	else
	{
		m_pos = data.pos;
		//m_target = data.target;
	}

	
	//データを受け取ってそれをもとにみるだけ
	
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	
}

void MainCamera::FixCameraPos()
{
}

void MainCamera::CameraSetting()
{
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}
void MainCamera::SetLerp(bool isLerp)
{
	auto player = m_cameraContext->m_player.lock();
	if (!player)return;

	//lerpするかをセット
	m_isLerp[0] = isLerp;
	m_isLerp[1] = isLerp;
	if (!m_isLerp[1])return;
	//目標地点をセット
	m_targetPos = m_cameraData.pos;
}


void MainCamera::SetSlerp(bool isSlerp)
{
	auto player = m_cameraContext->m_player.lock();
	if (!player)return;
	auto enemy = m_cameraContext->m_targetEnemy.lock();
	if (!enemy)return;
	
	//lerpするかをセット
	m_isSlerp[0] = isSlerp;
	m_isSlerp[1] = isSlerp;
	if (!m_isSlerp[1])return;
	//目標地点をセット
	m_targetPos = m_cameraData.pos;
	m_rotateAxis = enemy->GetPos(); //enemyの座標を回転軸にする

	//変化するベクトルの大きさを最初にセットする
	m_distance = (m_pos - m_rotateAxis).Magnitude();

}
