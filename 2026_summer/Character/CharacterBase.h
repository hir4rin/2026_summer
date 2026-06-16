#pragma once
#include "Collider.h"
#include "Animation.h"
#include <unordered_map>
#include <HitCol.h>

class Camera;
enum class Armor
{
	None = -1,
	Low,
	Middle,
	High,
};
enum class AttackPower
{
	None = -1,
	Low,
	Middle,
	High,
};

struct AttackData
{
	float attackPower;//攻撃力
	Vector3 knockBackPower;//攻撃が当たったときの吹き飛ばしの力
	float hitStopTime;//攻撃が当たったときのヒットストップの時間
	bool isKirimomi;//吹っ飛ぶかどうか
};

class CharacterBase abstract : public Collider
{
public:
	CharacterBase();
	virtual ~CharacterBase();

	void SetTimeScale(float timeScale) { m_ownTimeScale = timeScale; }
	float GetTimeScale() const { return m_ownTimeScale; }

	int GetModelHandle() const { return m_modelHandle; }
	Vector3 GetPos() const { return m_pos; }

protected:
	void UpdateAngleAndPos();//回転角度と座標の更新//


	int m_modelHandle = -1;//モデルのハンドル
	int m_hp = -1;//体力
	float m_rotAngleY = 0.0f;//回転角度
	float m_targetAngleY = 0.0f;//目標の回転角度//回転を滑らかにするためのもの
	Vector3 m_pos = {};//座標
	Vector3 m_vel = {};//速度

	Vector3 m_targetVec = {};//移動したい方向のベクトル
	//移動方向//カメラの向き
	Vector3 forward = Vector3(0, 0, 0);	
	Vector3 down = Vector3(0, 0, 0);
	Vector3 left = Vector3(0, 0, 0);
	Vector3 right = Vector3(0, 0, 0);

	Animation m_anim;//アニメーション(アニメーションブレンド、アニメーション再生など)
	std::unordered_map<std::string, std::string> m_animNames;//アニメーションの名前を管理
	const std::string& GetAnimName(const std::string& key)const;//アニメーションの名前を取得する

	HitCol m_hitcol;//やられ判定


	Armor m_armor = Armor::None;//アーマー
	AttackPower m_attackPower = AttackPower::None;//攻撃力

	float m_ownTimeScale =1.0f;//自分のtimeScale
	bool m_isGround = true;//地面にいるかどうか
	bool m_isHit = false;//攻撃を喰らったか
	bool m_isDead = false;//死んでいるか
	//参照用のカメラ
	Camera* m_camera;
};

