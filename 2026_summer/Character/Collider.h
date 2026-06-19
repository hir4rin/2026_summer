#pragma once
#include "../Math/Vector3.h"
#include "../Managers/CollisionManager.h"
#include "../IDManager.h"
#include <string>


enum class ColliderType
{
	Sphere,
	Box,
	Capsule
};
/// <summary>
/// タグ
/// </summary>
enum class Tags
{
	None = -1,
	StaticObject,//静的オブジェクト
	Player,
	PlayerHit,
	PlayerAttack,
	Enemy,
	EnemyHit,
	EnemyAttack,
};
/// <summary>
/// 位置補正の優先度
/// </summary>
enum class ColPriority
{
	//優先度が低いほど、位置補正の処理が後になる
	None = -1,
	Low = 0,
	Middle,
	High,
	Static,
};
struct VelocityInfo
{
	Vector3 vel = {};//速度
	float decayRate;//0.0f= 1frameで削除、0<x<1= 減衰、1.0f=減衰なし(持続)
};

/// <summary>
/// MathVector3をincludeしないとつかえない
/// </summary>
class Collider abstract
{
public:
	Collider();
	virtual ~Collider();
	void ColUpdate(Vector3 pos);//当たり判定の更新//継承先で使う

	//当たり判定をチェック
	bool IsCollidable(const Collider& other)const;
	//当たったときに呼ばれる関数
	virtual void OnCollision(Collider& other) = 0;

	//押し戻しの処理//この後、ColUpdateをして、更新すること
	Vector3 PushBack(Collider& other);


	//Actorが派生先ですること-------------------------------------------------------------
	//当たり判定の初期化処理//登録
	/// <summary>座標、中心点、半径、当たり判定のタイプ、タグ、当たり判定が有効かどうか</summary>
	void  ColInit(Vector3 pos,Vector3 offset, float radius, ColliderType type, Tags tag, bool isActive,bool isTrigger= false);
	//IDのセット
	void SetID();
	//--------------------------------------------------------------------------

	//セッター
	void SetCenter(const float center) { m_center = center; }
	void SetRadius(float radius) { m_radius = radius; }
	void SetType(ColliderType type) { m_type = type; }
	void SetTag(Tags tag) { m_tag = tag; }
	void SetIsActive(bool isActive) { m_isActive = isActive; }

	//ゲッター
	Collider* GetCollider() { return this; }
	float GetCenter()const { return m_center; }
	float GetRadius()const { return m_radius; }
	ColliderType GetType()const { return m_type; }
	Tags GetTag()const { return m_tag; }
	bool IsActive()const { return m_isActive;}
	int GetId()const { return m_id; }
	bool GetIsTrigger()const { return m_isTrigger; }
	Vector3 GetPos() const { return m_pos; }
	Vector3 GetVel() const { return m_vel; }
	Vector3 GetWorldCenter() const { return m_pos + m_offset; }//ワールド座標での中心位置を返す
	float GetTimeScale() const { return m_ownTimeScale; }
	//デバッグ描画
	void DebugDraw()const;
protected:
	virtual void ApplyPos()=0;
	void AddVelocity(const std::string& name, Vector3 vel, float decayRatew);//速度の追加//名前、速度、減衰率
protected:
	ColliderType m_type;
	Tags m_tag;
	float m_center;
	Vector3 m_pos = {};//座標
	Vector3 m_offset = {};//基準位置からのオフセット
	std::unordered_map<std::string, VelocityInfo> m_velocities;//速度のリスト//キーは名前、値は(速度,減衰率)
	Vector3 m_vel = {};//速度
	Vector3 m_knockBackVel = {};//吹き飛ばしのベクトル//ここに書きたくないが用途のため書く
	float m_radius;//Sphereの半径、Boxの幅、Capsuleの半径
	bool m_isActive;//当たり判定が有効かどうか
	bool m_isTrigger;//当たり判定のみをし、押し戻しなどはしない
	int m_id;//当たり判定などに使うID
	float m_ownTimeScale = 1.0f;//自分のtimeScale

	friend class CollisionManager;
};

