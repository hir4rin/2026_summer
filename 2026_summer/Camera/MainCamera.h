#pragma once
#include "Camera.h"
#include "../Math/Vector3.h"

struct CameraContext;

/// <summary>
//・抽象化が適切ではない
//・コンポーネント思考も勉強したほうがいい



//MainCameraはpriorityが高いものの座標や、注視点、角度をもらってそこをDxLibで見るだけの機能
//MainCameraはLerpでいどうするか、Slerpで移動するか(その場合、回転の中心点をもらう)
class MainCamera :public Camera
{
public:
	MainCamera();
	virtual ~MainCamera();

	void Init()override;
	void Draw()override;
	void Update(Vector3 pos, Vector3 pos2 = Vector3())override;
	void Update(const CameraData& data);
	void FixCameraPos()override;
	void CameraSetting()override;

	
	//第二引数で割合を指定してもいい//一旦パス
	void SetLerp(bool isLerp);
	void SetSlerp(bool isSlerp);

	//ロックオン
	void SetLockOn(bool isLockOn) { m_isLockOn = isLockOn; }
	bool GetIsLockOn()const { return m_isLockOn; }

private:
	bool m_isLerp[2] = { false ,false};//注視点と座標
	bool m_isSlerp[2] = {false,false};//注視点と座標
	bool m_isLockOn = false;//ロックオンしているかどうか

	float m_distance = 0.0f;//Slerpの時に使うベクトルの大きさ
	//Lerp,Slerpの目標到達座標
	Vector3 m_targetPos;
	Vector3 m_rotateAxis;//Slerpの回転軸//Y軸ではあるので、実質座標



	

};

