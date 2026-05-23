#pragma once
#include "../Math/Vector3.h"
#include "DxLib.h"
#include <memory>
class Input;
class Camera
{
public:
	enum class Type
	{
		None = -1,
		PlayerCamera = 0,
		Movie1Camera = 1,
		
		//他のカメラもここに追加していく
	};
	Camera();
	virtual ~Camera();

	void TitleInit();//タイトル画面での初期化

	virtual void Init();
	/// <summary>
	/// 
	/// </summary>
	/// <param name="input"></param>
	/// <param name="pos">プレイヤーの座標</param>
	virtual void Update(Vector3 pos,Vector3 pos2 = Vector3());
	void Draw();
	virtual void FixCameraPos();//カメラの位置を調整する
	Vector3 GetCameraPos()const { return m_pos; }

	int GetPriority()const { return m_priority; }
	void SetPriority(int priority) { m_priority = priority; }
	Type GetCameraType()const { return m_type; }
	bool GetIsChangeCamera()const { return m_isChangeCamera; }
	void SetIsChangeCamera(bool isChange) { m_isChangeCamera = isChange; }

	void SetCameraAngle(float angleH, float angleV) { m_angleH = angleH; m_angleV = angleV; }//引き渡しのときにカメラの角度を設定
	float GetCameraAngleH()const { return m_angleH; }
	float GetCameraAngleV()const { return m_angleV; }

		/// <summary>
	/// カメラを揺らす
	/// </summary>
	/// <param name="camera">カメラのポインタ</param>
	/// <param name="power">揺れの強さ</param>
	/// <param name="time">揺らす時間</param>
	void StartCameraShake(Camera& camera, float power, float time);
	Vector3 CameraShakeUpdate();
protected:
	void InputRightStick();//右スティックの入力を処理する
protected:
	int m_priority = 0;//カメラの優先度//複数のカメラがあるときに、どのカメラを優先するかを決めるためのもの//数値が大きいほど優先される
	bool m_isChangeCamera = false;//カメラを切り変えるかどうかのフラグ
	Vector3 m_target;//注視点
	Vector3 m_pos;//カメラの位置
	Type m_type = Type::None;//カメラの種類
	
	float m_angleH = 0.0f;// 水平方向の回転角度
	float m_angleV = 0.0f;// 垂直方向の回転角度

	//カメラ揺れ用
	float m_shakePower = 0.0f;
	float m_shakeTimer = 0.0f;
	float m_shakeTimerMax = 0.0f;//減衰用のコピー
	bool m_isShaking = false;//今カメラが揺れているかどうか
};

