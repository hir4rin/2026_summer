#include "PlayerFollowCamera.h"
#include "../Cameramanager.h"
#include "Player.h"
#include "../PlayerCamera.h"
#include "../Math/Matrix4x4.h"
#include "../Input.h"
#include "../CameraManager.h"
#include "../LockOnManager.h"
#include "../Character/Enemy/EnemyBase.h"
#include "../SubWindow/SubWindow.h"
#include "../Stage/Stage.h"
#include <algorithm>



namespace
{
	//constexpr int kNormalRx = 128;//右スティックの通常の値

	constexpr float kToPlayerLength = 700.0f;//プレイヤーからカメラまでの距離
	constexpr float kCameraHeightFloat = 300.0f;//カメラの高さ
	constexpr float kCameraAngleSpeed = 0.03f;//カメラの回転速度
	const Vector3 kCameraHeight = Vector3(0.0f, 150.0f, 0.0f);//カメラの高さ(Vector3)

	constexpr float kRockOnMaxDistance = 10000.0f;//ロックオンの最大距離

	constexpr float kGroundCheckDistance = 800.0f;//プレイヤーの最高到達点//カメラのターゲットの割合注視点
}

PlayerFollowCamera::PlayerFollowCamera(std::weak_ptr<CameraManager> owner):CameraStateBase(owner)
{
}

PlayerFollowCamera::~PlayerFollowCamera()
{
}

void PlayerFollowCamera::Enter(CameraData data)
{
	auto player = m_owner.lock()->GetContext()->m_player.lock();

	Vector3 playerPos = Vector3(player->GetPos().x, 0.0f, player->GetPos().z);

	m_target = playerPos + kCameraHeight;
	m_pos = data.pos;
	//カメラの位置を調整する
	FixCameraPos();
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void PlayerFollowCamera::Update()
{
	auto player = m_owner.lock()->GetContext()->m_player.lock();

	InputRightStick();

	//ターゲットの位置を更新
	Vector3 playerPos = player->GetPos();
	m_testPos = playerPos;
	//プレイヤーの位置と直下の地面との距離が一定以下の場合、カメラを動かさない
	Vector3 endPos = playerPos + Vector3(0.0f, -kGroundCheckDistance, 0.0f);
	m_testPos2 = endPos;
	auto stage = m_stage.lock();
	if (stage)
	{
		//ちょっと上から判定
		Vector3 startPos = playerPos + Vector3(0, 250, 0);
		//stage地面とプレイヤーの距離を取得する
		auto hitPoly = MV1CollCheck_Line(stage->GetStageModelHandle(), -1, startPos.ToDxLibVector(), endPos.ToDxLibVector());
		if (hitPoly.HitFlag)
		{
			float distance = playerPos.y - hitPoly.HitPosition.y;
			if (distance >= 0.0f)
			{
				//カメラ注視点の割合をきめる
				float ratio = distance / kGroundCheckDistance;
				playerPos.y *= ratio;
			}
			m_testPos2 = playerPos;
		}
	}
	//m_testPos2 = endPos;

	//rayVecを更新
	m_rayVec = playerPos - m_pos;
	//rayVecを正規化
	m_rayVec = m_rayVec.Normalize();
	//敵の座標との射影ベクトルを求める


	//playerPos.y = 0.0f;//プレイヤーのy座標は0にする
	m_focusGoal = playerPos + kCameraHeight;
	m_target = Vector3::Lerp(m_target, m_focusGoal, 0.5f);

	//カメラの位置を調整する
	FixCameraPos();

	//大きさによって、ラープかどうかを変える
	//Vector3 dis = m_targetPos - m_pos;
	//if (dis.Magnitude() < 0.15f)
	//{
	//	m_pos = m_targetPos;
	//}

	//Lerpの割合を上下差がある攻撃で変えたりす
	// るとよい
	m_pos = Vector3::Lerp(m_pos, m_targetPos, 0.5f);


	//カメラの位置と注視点を反映する
	//SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());

}

void PlayerFollowCamera::Exit()
{
}

void PlayerFollowCamera::FixCameraPos()
{
	//水平方向の回転
	auto rotY = Matrix4x4::MakeRotationY(m_angleH);
	auto rotX = Matrix4x4::MakeRotationX(m_angleV);

	//本当はこの回転行列はベクトルの量、角度が固定なのでOKだが、変わると回転量が変わるので危ないあぶない
	float cameraToPlayerLength = kToPlayerLength * 0.5f;

	//カメラの座標を算出
	auto CtoP = Vector3(0.0f, 0.0f, -cameraToPlayerLength);//プレイヤーからカメラへのベクトル
	//カメラの揺れを加える
	CtoP = CtoP + CameraShakeUpdate();
	//DxLibに変換
	auto CtoPVec = CtoP.ToDxLibVector();
	auto rotYMat = rotY.ToDxLibMatrix(rotY);//回転行列を転置する
	auto rotXMat = rotX.ToDxLibMatrix(rotX);//回転行列を転置する
	//カメラからプレイヤーVec
	auto RotCtoP = VTransform(CtoPVec, rotXMat);//回転させる
	RotCtoP = VTransform(RotCtoP, rotYMat);//回転させる

	auto pos = VAdd(RotCtoP, m_focusGoal.ToDxLibVector());//プレイヤーの座標に足す

	m_targetPos = Vector3::FromDxLibVector(pos);
}

void PlayerFollowCamera::CameraSetting()
{
	//FixCameraPos();
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void PlayerFollowCamera::InputRightStick()
{
	if (GetJoypadXInputState(DX_INPUT_PAD1, &xi) == 0)//右スティックの入力を取得できたら
	{
		//右スティックの範囲は-32768~35767
		short rx = xi.ThumbRX;
		short ry = xi.ThumbRY;
		//デッドゾーンの設定
		const int DeadZone = 8000;//スティックの傾きがこの値以下の場合は、傾いていないとみなす
		if (abs(rx) < DeadZone)
		{
			rx = 0;
		}
		if (abs(ry) < DeadZone)
		{
			ry = 0;
		}
		//スティックの傾きに応じてカメラを回転させる
		if (rx > 0)//右に傾いている
		{
			m_angleH += kCameraAngleSpeed;
			if (m_angleH > DX_PI_F * 2.0f)
			{
				m_angleH -= DX_PI_F * 2.0f;
			}
		}
		else if (rx < 0)//左に傾いている
		{
			m_angleH -= kCameraAngleSpeed;
			if (m_angleH < -DX_PI_F * 2.0f)
			{
				m_angleH += DX_PI_F * 2.0f;
			}
		}
		if (ry < 0)//下に傾いている
		{
			m_angleV += kCameraAngleSpeed * 1 / 3;
			// ある一定角度以上にはならないようにする
			if (m_angleV > DX_PI_F * 0.5f - 0.6f)
			{
				m_angleV = DX_PI_F * 0.5f - 0.6f;
			}

		}
		else if (ry > 0)//上に傾いている
		{
			m_angleV -= kCameraAngleSpeed * 1 / 3;
			// ある一定角度以下にはならないようにする
			if (m_angleV < -DX_PI_F * 0.4f + 0.6f)
			{
				m_angleV = -DX_PI_F * 0.4f + 0.6f;
			}

		}

	}
}
