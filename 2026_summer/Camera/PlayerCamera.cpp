#include "PlayerCamera.h"
#include "../Math/Matrix4x4.h"
#include "../Input.h"
#include "Player.h"
#include "CameraManager.h"
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
}


PlayerCamera::PlayerCamera() :
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
void PlayerCamera::Draw()
{/*
	if (m_isLockOn)
	{
		DrawLine3D(m_testPos.ToDxLibVector(), m_testPos2.ToDxLibVector(), GetColor(255, 255, 255));
	}*/
	std::string posText = "PlayerCameraPos:(" + std::to_string(m_pos.x) + "," + std::to_string(m_pos.y) + "," + std::to_string(m_pos.z) + ")";
	std::string targetText = "PlayerCameraTarget:(" + std::to_string(m_target.x) + "," + std::to_string(m_target.y) + "," + std::to_string(m_target.z) + ")";
	SubWindow::AddText(posText);
	SubWindow::AddText(targetText);
}

void PlayerCamera::Update(Vector3 pos, Vector3 pos2)
{
	//ロックオン問題点
	//・ロックオン時の角度が変

	//if (m_isLockOn)
	//{
	//	//
	//	auto enemy = m_lockOnEnemy.lock();
	//	if (enemy)
	//	{
	//		Vector3 enemyPos = enemy->GetPos();
	//		float distance = (enemyPos - pos).Magnitude();
	//		//ロックオンの最大距離を超えたらロックオンを解除する
	//		if (distance > kRockOnMaxDistance)
	//		{
	//			m_isLockOn = false;
	//			//解放
	//			m_lockOnEnemy.reset();
	//		}
	//		//ターゲットの位置を更新
	//		Vector3 playerPos = pos;
	//		m_target = playerPos + (enemyPos - playerPos) / 2 + kCameraHeight;
	//		//デバッグ用のポスを更新
	//		m_testPos = playerPos;
	//		m_testPos2 = enemyPos;
	//		//カメラの角度を更新
	//		//PlayerToEnemyVecから30度ずらす
	//		Vector3 PtoEVec = (enemyPos - playerPos).Normalize();
	//		//m_angleH =  
	//		
	//		//カメラの位置を調整する
	//		FixCameraPosLockOn();
	//		//Lerpでカメラの位置を更新する
	//		m_pos = Vector3::Lerp(m_pos, m_targetPos, 0.1f);

	//		//カメラの位置と注視点を反映する
	//		//SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());
	//		return;
	//	}
	//	else
	//	{
	//		m_isLockOn = false;
	//		//解放
	//		m_lockOnEnemy.reset();
	//	}
	//}


	InputRightStick();

	//ターゲットの位置を更新
	Vector3 playerPos = pos;
	//プレイヤーの位置と直下の地面との距離が一定以下の場合、カメラを動かさない
	Vector3 endPos = playerPos + Vector3(0.0f, -400.0f, 0.0f);
	auto stage = m_stage.lock();
	if (stage)
	{
		//stage地面とプレイヤーの距離を取得する
		auto hitPoly = MV1CollCheck_Line(stage->GetStageModelHandle(),-1,playerPos.ToDxLibVector(),endPos.ToDxLibVector());
		if(hitPoly.HitFlag)
			{
			float distance = (playerPos - Vector3::FromDxLibVector(hitPoly.HitPosition)).Magnitude();
			
		}
	}



	//rayVecを更新
	m_rayVec = playerPos - m_pos;
	//rayVecを正規化
	m_rayVec = m_rayVec.Normalize();
	//敵の座標との射影ベクトルを求める


	//playerPos.y = 0.0f;//プレイヤーのy座標は0にする
	m_target = playerPos + kCameraHeight;

	//カメラの位置を調整する
	FixCameraPos();
	
	//Lerpの割合を上下差がある攻撃で変えたりするとよい
	m_pos = Vector3::Lerp(m_pos, m_targetPos, 0.5f);
	

	//カメラの位置と注視点を反映する
	//SetCameraPositionAndTarget_UpVecY(m_pos.ToDxLibVector(), m_target.ToDxLibVector());

}

void PlayerCamera::FixCameraPos()
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

	auto pos = VAdd(RotCtoP, m_target.ToDxLibVector());//プレイヤーの座標に足す

	m_targetPos = Vector3::FromDxLibVector(pos);
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
void PlayerCamera::FixCameraPosLockOn()
{
	auto enemy = m_cameraContext->m_targetEnemy.lock();
	auto player = m_cameraContext->m_player.lock();
	if (!enemy)return;
	if (!player)return;

	//水平方向の回転//敵との距離によってこの角度を帰る
	auto rotY = Matrix4x4::MakeRotationY(0.2f);
	//auto rotX = Matrix4x4::MakeRotationX(0.16f);//固定
	
	//ここも敵との距離に寄って変える
	float cameraToPlayerLength = kToPlayerLength * 0.5f;

	//カメラの座標を算出
	//PtoEVecの逆ベクトルを15度程度ずらす、playerの座標から足す
	Vector3 playerPos = player->GetPos();
	Vector3 enemyPos = enemy->GetPos();
	//y座標を0にする
	playerPos.y = enemyPos.y = 0.0f;

	Vector3 EtoPVec = (playerPos - enemyPos).Normalize();
	EtoPVec *= cameraToPlayerLength * 2;


	//auto CtoP = Vector3(0.0f, 0.0f, -cameraToPlayerLength);//プレイヤーからカメラへのベクトル

	//カメラの揺れを加える
	//EtoPVec = EtoPVec + CameraShakeUpdate();
	//DxLibに変換
	auto EtoPVecDx = EtoPVec.ToDxLibVector();
	auto rotYMat = Matrix4x4::ToDxLibMatrix(rotY);//回転行列を転置する
	//auto rotXMat = Matrix4x4::ToDxLibMatrix(rotX);//回転行列を転置する

	//カメラからプレイヤーVec
	auto RotPtoC = VTransform(EtoPVecDx, rotYMat);//回転させる
	//RotPtoC = VTransform(RotPtoC, rotXMat);//回転させる
	RotPtoC = VAdd(RotPtoC, VGet(0.0f, 160.0f, 0.0f));
	Vector3 pospl = player->GetPos();
	pospl.y = 0.0f;

	//水平方向はその向き、垂直は初期化
	m_angleH = atan2f(RotPtoC.x, RotPtoC.z) + DX_PI_F;
	m_angleV = 0;


	auto pos = VAdd(RotPtoC, player->GetPos().ToDxLibVector());//プレイヤーの座標に足す

	m_targetPos = Vector3::FromDxLibVector(pos);
}
