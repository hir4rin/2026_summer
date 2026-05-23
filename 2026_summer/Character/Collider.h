#pragma once
#include "../Math/Vector3.h"
#include "../Managers/CollisionManager.h"
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
	Player,
	PlayerAttack,
	Enemy,
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
	bool IsCollible(const Collider& other)const;
	//当たったときに呼ばれる関数
	virtual void OnCollision(Collider& other) = 0;
	//押し戻しの処理//この後、ColUpdateをして、更新すること
	Vector3 PushBack(Collider& other);

	//当たり判定の初期化処理//登録
	/// <summary>中心点、半径、当たり判定のタイプ、タグ、当たり判定が有効かどうか</summary>
	void  ColInit(const Vector3& center, float radius, ColliderType type, Tags tag, bool isActive);

	//セッター
	void SetCenter(const Vector3& center) { m_center = center; }
	void SetRadius(float radius) { m_radius = radius; }
	void SetType(ColliderType type) { m_type = type; }
	void SetTag(Tags tag) { m_tag = tag; }
	void SetIsActive(bool isActive) { m_isActive = isActive; }

	//ゲッター
	Collider* GetCollider() { return this; }
	Vector3 GetCenter()const { return m_center; }
	float GetRadius()const { return m_radius; }
	ColliderType GetType()const { return m_type; }
	Tags GetTag()const { return m_tag; }
	bool isActive()const { return m_isActive;}

	//デバッグ描画
	void DebugDraw()const;
protected:
	ColliderType m_type;
	Tags m_tag;
	Vector3 m_center;
	float m_radius;//Sphereの半径、Boxの幅、Capsuleの半径
	bool m_isActive;//当たり判定が有効かどうか

};

