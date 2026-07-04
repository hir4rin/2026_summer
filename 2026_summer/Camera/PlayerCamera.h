#pragma once
#include "Camera.h"

class Player;
class EnemyBase;

class PlayerCamera : public Camera
{
public:
	PlayerCamera();
	virtual ~PlayerCamera();

	void Init()override;
	void GameSceneInit();//ゲームシーンでの初期化
	void Draw()override;
	 /// <summary>
	 /// pos:プレイヤーの座標
	 /// pos2:補助的な座標（必要に応じて使用）
	 /// </summary>
	 /// <param name="pos"></param>
	 /// <param name="pos2"></param>
	 void Update(Vector3 pos, Vector3 pos2 = Vector3()) override;
	 void FixCameraPos() override;
	 void CameraSetting() override;



private:
	void InputRightStick();//右スティックの入力を処理する
	void FixCameraPosLockOn();//ロックオン時のカメラの位置を調整する
private:
	XINPUT_STATE  xi;
	Vector3 m_rayVec = Vector3(0, 0, 0);//カメラの前方向のベクトル//カメラの注視点を決めるために使う　
	//lerp用
	Vector3 m_targetPos;//カメラの目標座標

	Vector3 m_testPos = Vector3(0, 0, 0);
	Vector3 m_testPos2 = Vector3(0, 0, 0);

};

