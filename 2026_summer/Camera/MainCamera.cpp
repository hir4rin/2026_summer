#include "MainCamera.h"
#include "CameraManager.h"
#include "Player.h"
#include "../Character/Enemy/EnemyBase.h"
#include "../SubWindow/SubWindow.h"

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

	//ラープだったらラープ、SlerpだったらSlerpする
	//まずはラープなしで動かす
	if (m_isLerp)
	{
		////現在座標から目標までの保管をしながらm_posを計算する
		m_pos = Vector3::Lerp(m_pos, data.pos, 0.1f);
		
		m_target = data.target;
	}
	else if (m_isSlerp)
	{

		//方向はSlerpで、距離はLerpでやる
		//m_distance;
		float distanceTarget = (m_targetPos - m_rotateAxis).Magnitude();

		Vector3 dirPrev = (m_pos - m_rotateAxis).Normalize();
		Vector3 dirTarget = (m_targetPos - m_rotateAxis).Normalize();

		m_distance = std::lerp(m_distance, distanceTarget, 0.1f);
		//現在座標から目標までの保管をしながらm_posを計算する
		m_pos = Vector3::Slerp(dirPrev, dirTarget, 0.1f) * m_distance + m_rotateAxis;
		m_target = data.target;
	}
	else
	{
		m_pos = data.pos;
		m_target = data.target;
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
	m_isLerp = isLerp;
	if (!m_isLerp)return;
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
	m_isSlerp = isSlerp;
	if (!m_isSlerp)return;
	//目標地点をセット
	m_targetPos = m_cameraData.pos;
	m_rotateAxis = enemy->GetPos(); //プレイヤーの座標を回転軸にする

	//変化するベクトルの大きさを最初にセットする
	m_distance = (m_pos - m_rotateAxis).Magnitude();

}
