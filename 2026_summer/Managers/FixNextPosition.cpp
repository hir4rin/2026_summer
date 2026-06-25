#include "FixNextPosition.h"
#include "FixNextPosition.h"
#include "../Character/Collider.h"
#include "../Stage/Stage.h"

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
