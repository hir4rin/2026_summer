#pragma once

class Collider;

class CollisionChecker
{
public:
	CollisionChecker();
	virtual ~CollisionChecker();
	/// <summary>
	/// 当たり判定の結果を返す
	/// </summary>
	/// <param name="colliderA"></param>
	/// <param name="colliderB"></param>
	/// <returns></returns>
	bool IsCollide(Collider& colliderA, Collider& colliderB);
private:

	/// <summary>
	/// 球と球の当たり判定
	/// </summary>
	/// <param name="colliderA"></param>
	/// <param name="colliderB"></param>
	/// <returns></returns>
	bool ChecCollSS(Collider& colliderA, Collider& colliderB);
	/// <summary>
	/// カプセルと球の当たり判定
	/// </summary>
	/// <param name="colliderA"></param>
	/// <param name="colliderB"></param>
	/// <returns></returns>
	bool CheckCollCS(Collider& colliderA, Collider& colliderB);

	/// <summary>
	/// カプセルとカプセルの当たり判定
	/// </summary>
	/// <param name="colliderA"></param>
	/// <param name="colliderB"></param>
	/// <returns></returns>
	bool CheckCollCC(Collider& colliderA, Collider& colliderB);


};

