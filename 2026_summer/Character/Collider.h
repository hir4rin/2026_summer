#pragma once
#include "../Math/Vector3.h"
#include "../Managers/CollisionManager.h"
#include "../IDManager.h"
#include <string>
#include <memory>

class Stage;

enum class ColliderType
{
	Sphere,
	Box,
	Capsule,
	Polygon,
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
	PlayerUltAttack,
	Enemy = 5,
	EnemyHit,
	EnemyAttack,
	Boss,
	WaveArea,
	Mascot
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

struct CapsuleInfo
{
	//自身の座標とendPosの2点でカプセルを作る
	Vector3 endPos;
	//最近点
	Vector3 hitNearestPos;
	//最近点からの距離
	float hitNearestDistance;
};

struct BoxInfo
{
	Vector3 halfExtents;//半分の大きさ//x,y,z
};


/// <summary>
/// MathVector3をincludeしないとつかえない
/// </summary>
class Collider abstract : public std::enable_shared_from_this<Collider>
{
public:
	Collider();
	virtual ~Collider();
	void ColUpdate(Vector3 pos = Vector3());//当たり判定の更新//継承先で使う

	//当たり判定をチェック
	bool IsCollidable(const Collider& other)const;
	//当たったときに呼ばれる関数
	virtual void OnCollision(Collider& other) = 0;

	//押し戻しの処理//この後、ColUpdateをして、更新すること
	Vector3 PushBack(Collider& other);

	virtual void OnTriggerEnter(Collider& other);//トリガー処理//当たり判定のみで、押し戻しなどはしない
	virtual void OnTriggerExit(Collider& other);//トリガーから離れたときの処理


	//Actorが派生先ですること-------------------------------------------------------------
	//当たり判定の初期化処理//登録
	/// <summary>座標、中心点、半径、当たり判定のタイプ、タグ、当たり判定が有効かどうか</summary>
	void  ColInit(Vector3 pos,Vector3 offset, float radius, ColliderType type, Tags tag, bool isActive,bool isTrigger= false, float lifeTime = 0.0f);
	//IDのセット//
	void SetID();
	void ResetID(int ownerID);//コライダーを持っている場合は、その親のidをセットする
	void SetStagePtr(std::weak_ptr<Stage> stage) { m_stage = stage; }//ステージへの弱参照をセット
	//--------------------------------------------------------------------------

	//ID・自身への参照
	std::shared_ptr<Collider> GetCollider() { return shared_from_this(); }
	int GetId()const { return m_id; }

	//タイプ・タグ
	void SetType(ColliderType type) { m_type = type; }
	ColliderType GetType()const { return m_type; }
	void SetTag(Tags tag) { m_tag = tag; }
	Tags GetTag()const { return m_tag; }

	//有効・トリガー
	void SetIsActive(bool isActive) { m_isActive = isActive; }
	bool IsActive()const { return m_isActive;}
	bool GetIsTrigger()const { return m_isTrigger; }

	//地面・壁への接触
	void SetIsFloor(bool isFloor) { m_isFloor = isFloor;}
	bool IsFloor()const { return m_isFloor;}
	void SetIsWall(bool isWall) { m_isWall = isWall;}
	bool IsWall()const { return m_isWall;}

	//座標・速度
	Vector3 GetPos() const { return m_pos; }
	Vector3 GetWorldCenter() const { return m_pos + m_offset; }//ワールド座標での中心位置を返す
	Vector3 GetNextPos() const { return GetWorldCenter() + m_vel; }//次のフレームでの座標を返す
	Vector3 GetVel() const { return m_vel; }
	void SetCenter(const float center) { m_center = center; }
	float GetCenter()const { return m_center; }

	//半径(Sphere・Capsuleで使用)
	void SetRadius(float radius) { m_radius = radius; }
	float GetRadius()const { return m_radius; }

	//BOX
	void SetBoxHalfExtents(Vector3 halfExtents) { m_boxInfo.halfExtents = halfExtents; }//Boxの半径
	Vector3 GetBoxHalfExtents() const { return m_boxInfo.halfExtents; }

	//CAPSULE
	void SetCapsuleEndPos(Vector3 endPos) { m_capsuleInfo.endPos = endPos; }//カプセルの終点の座標をセット//最初に使う
	Vector3 GetCapsuleEndPos() const { return m_capsuleInfo.endPos; }//カプセルの終点の座標を返す

	//寿命
	void SetLifeTimeLimited() { m_isLifeTimeLimited = true; }
	bool GetIsLifeTimeLimited() const { return m_isLifeTimeLimited; }

	//タイムスケール
	void SetOwnTimeScale(float timeScale, float time) { m_ownTimeScale = timeScale; m_timeCounter = time; }
	float GetTimeScale() const { return m_ownTimeScale; }


	//デバッグ描画
	void DebugDraw()const;
protected:
	virtual void ApplyPos()=0;
	//void AddVelocity(const std::string& name, Vector3 vel, float decayRatew);//速度の追加//名前、速度、減衰率
protected:
	ColliderType m_type;
	Tags m_tag;
	float m_center;

	Vector3 m_pos = {};//座標
	Vector3 m_offset = {};//基準位置からのオフセット
	//std::unordered_map<std::string, VelocityInfo> m_velocities;//速度のリスト//キーは名前、値は(速度,減衰率)
	Vector3 m_vel = {};//速度
	Vector3 m_knockBackVel = {};//吹き飛ばしのベクトル//ここに書きたくないが用途のため書く

	float m_radius;//Sphereの半径、Boxの幅、Capsuleの半径
	bool m_isActive;//当たり判定が有効かどうか

	bool m_isTrigger;//当たり判定のみをし、押し戻しなどはしない
	bool m_isFloor;//床に当たったかどうか
	bool m_isWall;//壁に当たったかどうか
	int m_id;//当たり判定などに使うID
	float m_lifeTime = 0.0f;//寿命//0.0fで無限//1.0fで1秒
	bool m_isLifeTimeLimited = false;//trueになったらCollisionManagerから削除される
	float m_ownTimeScale = 1.0f;//自分のtimeScale
	float m_timeCounter = 0.0f;//timeScaleのカウンター
	CapsuleInfo m_capsuleInfo = {};//カプセルの情報//カプセルの時に使う
	BoxInfo m_boxInfo = {};//ボックスの情報//ボックスの時に使う

	std::weak_ptr<Stage> m_stage;//ステージへの弱参照//ステージとのレイキャスト用

	std::vector<std::weak_ptr<Collider>> m_currentPressColliders;///現在触れているコライダーのリスト
	std::vector<std::weak_ptr<Collider>> m_prevPressColliders;///前のフレームで触れていたコライダーのリスト

	friend class CollisionManager;
	friend class CollisionChecker;
	friend class FixNextPosition;
};

