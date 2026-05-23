#pragma once
#include "Camera.h"
class PlayerCamera : public Camera
{
public:
	PlayerCamera();
	virtual ~PlayerCamera();

	void Init()override;
	void GameSceneInit();//ゲームシーンでの初期化
	 /// <summary>
	 /// pos:プレイヤーの座標
	 /// pos2:補助的な座標（必要に応じて使用）
	 /// </summary>
	 /// <param name="pos"></param>
	 /// <param name="pos2"></param>
	 void Update(Vector3 pos, Vector3 pos2 = Vector3()) override;
	 void FixCameraPos() override;
private:
	void InputRightStick();//右スティックの入力を処理する
private:
	XINPUT_STATE  xi;
};

