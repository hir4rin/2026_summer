#include "Collider.h"
#include "../IDManager.h"

Collider::Collider():
	m_type(ColliderType::Sphere),
	m_tag(Tags::None),
	m_center(0.0f),
	m_radius(1.0f),
	m_isActive(true)
{
}

Collider::~Collider()
{
	//コライダーをコリジョンマネージャーから解除する
	CollisionManager::GetInstance().ReleaseCollider(this);
}

void Collider::ColUpdate(Vector3 pos)
{
	////球の場合のみ//いったん
	//Vector3 offset = Vector3(0, 50.0f, 0);//プレイヤーのモデルの中心は足元にあるので、当たり判定の中心をプレイヤーのモデルの中心から少し上にするためのオフセット
	//if (m_type == ColliderType::Sphere)
	//{
	//	SetCenter(pos + offset);
	//}
}

bool Collider::IsCollidable(const Collider& other) const
{
	//球と球の当たり判定を実装予定
	Vector3 Apos = GetWorldCenter();//自分の当たり判定の中心の座標
	Vector3 Bpos = other.GetWorldCenter();//相手の当たり判定の中心の座標

	//2点間の距離の2乗を求める
	float sqDistance = (Apos - Bpos).sqMagnitude();
	//半径の和の2乗を求める
	float sqRadiusSum = (GetRadius() + other.GetRadius()) * (GetRadius() + other.GetRadius());

	if (sqDistance <= sqRadiusSum)
	{
		return true;
	}
	return false;
}

Vector3 Collider::PushBack(Collider& other)
{
	//otherから自分へのベクトル//velを足した値
	Vector3 centerA = GetWorldCenter() + m_vel;//自分の当たり判定の中心の座標
	Vector3 centerB = other.GetWorldCenter() + other.m_vel;//相手の当たり判定の中心の座標

	auto oToMeVec = (centerA - centerB);
	oToMeVec.y = 0.0f;//Y軸の成分を0にする//水平面でのベクトルにする
	float distance = oToMeVec.Magnitude();
	//距離が0の時は当たり判定をしない
	if (distance == 0)return Vector3(0, 0, 0);
	//重なりの深さ　＝　（自分の半径＋相手の半径）－距離
	float overlap = GetRadius() + other.GetRadius() - distance;
	if (overlap > 0)
	{
		return oToMeVec.Normalize() * (overlap*0.5f);// 重なった分だけ押し戻す（0.5倍にして両方が半分ずつ退く）
	}
	//重なっていない場合は、押し戻さない
	return Vector3(0, 0, 0);
}

void Collider::ColInit(Vector3 pos, Vector3 offset, float radius, ColliderType type, Tags tag, bool isActive,bool isTrigger)
{
	m_pos = pos;
	m_offset = offset;//m_posからのoffset
	SetRadius(radius);
	SetType(type);
	SetTag(tag);
	SetIsActive(isActive);
	m_isTrigger = isTrigger;
	//コライダーをコリジョンマネージャーに登録する
	CollisionManager::GetInstance().RegisterCollider(this);
}

void Collider::SetID()
{
	m_id = IDManager::GetNextID();
}

void Collider::DebugDraw() const
{
	//非アクティブなら描画しない
	if(!m_isActive)return;
	
	//タグによって色を変える
	unsigned int color = GetColor(0, 0, 0);//デフォルトは黒

	switch (m_tag)
	{
		case Tags::Player:
			color = GetColor(0, 255, 0);//プレイヤーは緑
			break;
		case Tags::Enemy:
			color = GetColor(128, 128, 128);//敵は灰色
			break;
		case Tags::PlayerAttack:
			color = GetColor(0, 0, 255);//プレイヤーの攻撃は青
			break;
		default:
			break;
	}
	//タイプによって描画を分ける
	switch (m_type)
	{
		case ColliderType::Sphere:
			DrawSphere3D(GetWorldCenter().ToDxLibVector(), m_radius, 16, color, color, false);
			break;
		case ColliderType::Box:
			//Boxの描画は、中心と幅から、8点の頂点を求めて、そこから線を引いて描画する
			break;
		case ColliderType::Capsule:
			//Capsuleの描画は、球と円柱を組み合わせて描画する
			break;
		default:
			break;
	}
}
