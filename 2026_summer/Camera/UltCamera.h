#pragma once
#include "Camera.h"

class Player;
class EnemyBase;

class UltCamera : public Camera
{
public:
	UltCamera();
	virtual ~UltCamera();
	void Init()override;
	void Update(Vector3 pos, Vector3 pos2 = Vector3()) override;
	void FixCameraPos() override;
	void CameraSetting() override;
	void Draw() override;
	void SetCameraData(const CameraData& data) override { m_cameraData = data; Init(); }
private:
	//lerp用
	Vector3 m_targetPos;//カメラの目標座標
	float m_VecLength = 0.0f;//カメラの目標座標までの大きさ

	//なんかもっといい設計ありそうだけどいったんこれで
	std::weak_ptr<Player> m_player;//プレイヤーの弱参照//ロックオン用
	std::weak_ptr<EnemyBase> m_lockOnEnemy;//ロックオンしている敵の弱参照//していなかったら攻撃時の内部ターゲットのやつにする
};

