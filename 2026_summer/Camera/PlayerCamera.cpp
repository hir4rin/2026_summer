#include "PlayerCamera.h"
#include "../Math/Matrix4x4.h"

namespace
{
	//constexpr int kNormalRx = 128;//右スティックの通常の値

	constexpr float kToPlayerLength = 700.0f;//プレイヤーからカメラまでの距離
	constexpr float kCameraHeightFloat = 300.0f;//カメラの高さ
	constexpr float kCameraAngleSpeed = 0.03f;//カメラの回転速度
	const Vector3 kCameraHeight = Vector3(0.0f, 150.0f, 0.0f);//カメラの高さ(Vector3)
}


PlayerCamera::PlayerCamera():
	xi{}
{
	m_type = Type::PlayerCamera;
	m_priority = 10;//プレイヤーカメラは優先度が高い
	m_pos = Vector3(0.0f, kCameraHeightFloat, -kToPlayerLength);
	m_target = Vector3(0.0f, 0.0f, 0.0f);
}

PlayerCamera::~PlayerCamera()
{
}

void PlayerCamera::Init()
{
	// カメラの設定
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraNearFar(100.0f, 2500.0f);
	GameSceneInit();
}

void PlayerCamera::GameSceneInit()
{
	//ターゲットの位置を更新
	Vector3 playerPos = Vector3();
	playerPos.y = 0.0f;//プレイヤーのy座標は0にする
	m_target = playerPos + kCameraHeight;

	//カメラの位置を調整する
	FixCameraPos();
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void PlayerCamera::Update(Vector3 pos, Vector3 pos2)
{
	InputRightStick();

	//ターゲットの位置を更新
	Vector3 playerPos = pos;
	//playerPos.y = 0.0f;//プレイヤーのy座標は0にする
	m_target = playerPos + kCameraHeight;

	//カメラの位置を調整する
	FixCameraPos();
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());

}

void PlayerCamera::FixCameraPos()
{
	//水平方向の回転
	auto rotY = Matrix4x4::MakeRotationY(m_angleH);
	auto rotX = Matrix4x4::MakeRotationX(m_angleV);

	float cameraToPlayerLength = kToPlayerLength * 0.5f;

	//カメラの座標を算出
	auto CtoP = Vector3(0.0f, 0.0f, -cameraToPlayerLength);//プレイヤーからカメラへのベクトル
	//カメラの揺れを加える
	CtoP = CtoP + CameraShakeUpdate();
	//DxLibに変換
	auto CtoPVec = CtoP.ToDxLibVector();
	auto rotYMat = rotY.ToDxLibMatrix(rotY);//回転行列を転置する
	auto rotXMat = rotX.ToDxLibMatrix(rotX);//回転行列を転置する

	auto RotCtoP = VTransform(CtoPVec, rotXMat);//回転させる
	RotCtoP = VTransform(RotCtoP, rotYMat);//回転させる

	auto pos = VAdd(RotCtoP, m_target.ToDxLibVector());//プレイヤーの座標に足す

	m_pos = Vector3::FromDxLibVector(pos);
}

void PlayerCamera::CameraSetting()
{
	//FixCameraPos();
	//カメラの位置と注視点を反映する
	SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
}

void PlayerCamera::InputRightStick()
{
	if (GetJoypadXInputState(DX_INPUT_PAD1, &xi) == 0)//右スティックの入力を取得できたら
	{
		//右スティックの範囲は-32768~35767
		short rx = xi.ThumbRX;
		short ry = xi.ThumbRY;
		//スティックの傾きの値
		//DrawFormatString(0, 32, GetColor(255, 255, 255), "RX:%d", rx);
		//DrawFormatString(0, 48, GetColor(255, 255, 255), "RY:%d", ry);
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
			m_angleV += kCameraAngleSpeed*1/3;
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
