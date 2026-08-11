#include "TitleCamera.h"
#include "../SubWindow/SubWindow.h"

namespace
{
	constexpr float kToOriginLength = 200.0f;//原点からカメラまでの距離
	constexpr float kCameraHeight = 100.0f;//カメラの高さ

	const Vector3 kFollowCameraOffset = Vector3(0.0f, 80.0f, -150.0f);//追従時の、プレイヤーから見たカメラの相対位置
	const Vector3 kFollowTargetOffset = Vector3(0.0f, 50.0f, 0.0f);//追従時の、プレイヤーから見た注視点の相対位置//足元を見せる
}

TitleCamera::TitleCamera()
{
	m_type = Type::TitleCamera;
	m_priority = 10;//タイトル画面ではこのカメラしか使わない
	m_pos = Vector3(0.0f, kCameraHeight, -kToOriginLength);
	m_target = Vector3(0.0f, 100.0f, 0.0f);//原点を見る
}

TitleCamera::~TitleCamera()
{
}

void TitleCamera::Init()
{
	//カメラの設定
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraNearFar(0.0f, 1500.0f);
}

void TitleCamera::Draw()
{
	const std::string posText = "TitleCameraPos:(" + std::to_string(m_pos.x) + "," + std::to_string(m_pos.y) + "," + std::to_string(m_pos.z) + ")";
	SubWindow::AddText(posText);
}

void TitleCamera::Update(Vector3 pos, Vector3 pos2)
{
	switch (m_shot)
	{
	case Shot::Fixed:
		//定点で映すだけなので、座標・注視点は固定で更新の必要はない
		m_pos = Vector3(0.0f, kCameraHeight, -kToOriginLength);
		m_target = Vector3(0.0f, 100.0f, 0.0f);//原点を見る

		m_pos = pos + kFollowCameraOffset + Vector3(0,100,-100.0f);
		m_target = pos + kFollowTargetOffset+Vector3(0,100,0);
		break;
	case Shot::FollowPlayer:
		//プレイヤーとの相対位置を保ったまま追従する
		m_pos = pos + kFollowCameraOffset;
		m_target = pos + kFollowTargetOffset;
		break;
	}
}

void TitleCamera::CameraSetting()
{
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}
