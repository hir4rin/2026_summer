#include "FixNextPosition.h"
#include "FixNextPosition.h"
#include "../Character/Collider.h"
#include "../Stage/Stage.h"
#include <algorithm>

namespace
{
	//押し戻しの値に足して密着するのを防ぐ
	constexpr float kOverlapGap = 1.0f;
	constexpr float kCheckUnder = -800.0f;
	constexpr float kCheckTop = 800.0f;
	constexpr float kWallThreshold = 0.5f;//境界線
}



FixNextPosition::FixNextPosition()
{
}

FixNextPosition::~FixNextPosition()
{
}

void FixNextPosition::FixNextPos(Collider& colA, Collider& colB)
{
	//初期化
	m_wall.clear();
	m_floorAndRoof.clear();

	//コライダーのタイプによって処理を分ける
	auto typeA = colA.GetType();
	auto typeB = colB.GetType();
	//球と
	if(typeA == ColliderType::Sphere)
	{
		//球
		if(typeB == ColliderType::Sphere)
		{
			FixNextPosSS(colA, colB);
		}
		//カプセル
		else if(typeB == ColliderType::Capsule)
		{
			FixNextPosCS(colA, colB);
		}
		//ポリゴン
		else if(typeB == ColliderType::Polygon)
		{
			FixNextPosSP(colA, colB);
		}
	}
	//カプセルと
	else if (typeA == ColliderType::Capsule)
	{
		//球
		if (typeB == ColliderType::Sphere)
		{
			FixNextPosCS(colA, colB);
		}
		//カプセル
		else if (typeB == ColliderType::Capsule)
		{
			FixNextPosCC(colA, colB);
		}
		//ポリゴン
		else if (typeB == ColliderType::Polygon)
		{
			FixNextPosCP(colA, colB);
		}
	}
	//ポリゴンと
	else if (typeA == ColliderType::Polygon)
	{
		//球
		if (typeB == ColliderType::Sphere)
		{
			FixNextPosSP(colA, colB);
		}
		//カプセル
		else if (typeB == ColliderType::Capsule)
		{
			FixNextPosCP(colA, colB);
		}
	}
}

void FixNextPosition::FixNextPosSS(Collider& colA, Collider& colB)
{
	//otherから自分へのベクトル//velを足した値
	Vector3 centerA = colA.GetWorldCenter() + colA.GetVel();//自分の当たり判定の中心の座標
	Vector3 centerB = colB.GetWorldCenter() + colB.GetVel();//相手の当たり判定の中心の座標

	auto AToBVec = (centerB - centerA);
	AToBVec.y = 0.0f;//Y軸の成分を0にする//水平面でのベクトルにする
	float distance = AToBVec.Magnitude();

	//距離が0の時は押し戻しをしない
	if (distance == 0)return;

	//重なりの深さ　＝　（自分の半径＋相手の半径）－距離
	float overlap = colA.GetRadius() + colB.GetRadius() - distance;
	if (overlap > 0)
	{
		colA.m_vel += AToBVec.Normalize() * overlap * 0.5f;// 重なった分だけ押し戻す（0.5倍にして両方が半分ずつ退く） 
		colB.m_vel +=  AToBVec.Normalize() * overlap * 0.5f * -1;// 重なった分だけ押し戻す（0.5倍にして両方が半分ずつ退く）
		return;
	}
	//重なっていない場合は、押し戻さない
	return;
}

void FixNextPosition::FixNextPosSP(Collider& colA, Collider& colB)
{
	auto polygonCol = dynamic_cast<Stage*>(&colB);

	//当たったポリゴンの情報
	auto& hidDim = polygonCol->GetHitDim();

	//球の座標
	Vector3 posA = colA.GetNextPos();

	//床ポリゴンと壁ポリゴンに分ける
	AnalyzeWallAndFloor(hidDim, posA);
	
	//床か天井に当たったか
	bool isFloorAndRoofHit = !m_floorAndRoof.empty();
	//壁に当たったか
	bool isWall = !m_wall.empty();

	//床と当たったなら
	if (isFloorAndRoofHit)
	{
		//補正するベクトルを返す
		Vector3 overlapVec = 


	}

}

Vector3 FixNextPosition::OverlapVec(const Vector3& nextPos, std::vector<MV1_COLL_RESULT_POLY>& dim, float shortDistance)
{
	//垂線を下ろして近い点を探して祭壇距離を求める
	float hitShortDis = FLT_MAX;//最短距離//FLT_MAXはfloat型の最大値
	//法線
	Vector3 normal = {};
	for (auto& poly : dim)
	{
		//内積と法線ベクトルからあたっている座標を求める//射影ベクトルの計算
		Vector3 PolyToPos = nextPos - Vector3::FromDxLibVector(poly.Position[0]);
		float dot = PolyToPos.Dot(Vector3::FromDxLibVector(poly.Normal));

		//ポリゴンと当たったオブジェクトが法線方向にいるなら向きを反転//???
		//dotではposからのベクトルなので、hitPosを求めるために反転させる必要がある
		if ((PolyToPos.y > 0 && poly.Normal.y > 0) || (PolyToPos.y < 0 && poly.Normal.y < 0))
		{
			//ベクトルと法線が同じ向きなら反転が必要
			dot *= -1;
		}
		//当たった座標
		Vector3 hitPos = Vector3::FromDxLibVector(poly.Normal) * dot + nextPos;
		//距離
		float dis = (hitPos - nextPos).Magnitude();

		//球の半径より遠い場合は無視
		if(dis > shortDistance)
		{
			continue;
		}
		//初回または前回より距離が短いなら
		if (hitShortDis > dis)
		{
			hitShortDis = dis;
			normal = Vector3::FromDxLibVector(poly.Normal);
		}
	}
	//押し戻し
	//どれくらい押し戻すのか
	float overlap = shortDistance - hitShortDis;
	overlap = std::clamp(overlap, 0.0f, shortDistance);
	overlap += kOverlapGap;

	return normal * overlap;
}
