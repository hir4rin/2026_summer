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
	void PlayerSet(std::weak_ptr<Player> player) { m_player = player; }//プレイヤーの弱参照を設定する
	void GameSceneInit();//ゲームシーンでの初期化
	 /// <summary>
	 /// pos:プレイヤーの座標
	 /// pos2:補助的な座標（必要に応じて使用）
	 /// </summary>
	 /// <param name="pos"></param>
	 /// <param name="pos2"></param>
	 void Update(Vector3 pos, Vector3 pos2 = Vector3()) override;
	 void FixCameraPos() override;
	 void CameraSetting() override;
	 void SetLockOnEnemy(std::weak_ptr<EnemyBase> enemy) { m_isLockOn = true; m_lockOnEnemy = enemy; }//ロックオンする敵を設定
	 void ReleaseLockOnEnemy() { m_isLockOn = false; m_lockOnEnemy.reset(); }//ロックオンを解除する
	 bool GetIsLockOn()const { return m_isLockOn; }//ロックオンしているかどうかを返す
private:
	void InputRightStick();//右スティックの入力を処理する
private:
	XINPUT_STATE  xi;
	Vector3 m_rayVec = Vector3(0, 0, 0);//カメラの前方向のベクトル//カメラの注視点を決めるために使う　
	//ロックオン用
	bool m_isLockOn = false;//ロックオンしているかどうか
	std::weak_ptr<Player> m_player;//プレイヤーの弱参照//ロックオン用
	std::weak_ptr<EnemyBase> m_lockOnEnemy;//ロックオンしている敵の弱参照
};

