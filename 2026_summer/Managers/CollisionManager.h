#pragma once
#include <vector>
#include <functional>
#include <memory>

class Collider;

//衝突情報
struct CollisionInfo
{
	Collider* colliderA;//衝突したコライダーA
	Collider* colliderB;//衝突したコライダーB
};
//衝突時のコールバック関数の型//????//わからないから使わない
using CollisionCallback = std::function<void(const CollisionInfo&)>;

class CollisionManager
{
public:
	//シングルトンインスタンスを取得
	static CollisionManager& GetInstance()
	{
		static CollisionManager instance;
		return instance;
	}
	//コライダーの登録・解除
	void RegisterCollider(Collider* collider);
	void ReleaseCollider(Collider* collider);
	//すべてのコライダーをクリア
	void Clear();
	//当たり判定の一括チェック
	void Update();
	//デバッグ描画
	void DebugDraw()const;
private:
	//コンストラクタとデストラクタをプライベートにして、シングルトンパターンを実装
	CollisionManager() = default;
	virtual ~CollisionManager() = default;
	//コピーコンストラクタと代入演算子を削除して、シングルトンのインスタンスが複製されないようにする
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;

	//すべてのコライダー
	std::vector<Collider*> m_colliders;
	//std::vector<std::pair<Collider::Tags, CollisionCallback>> m_callbacks;//衝突時のコールバック関数のリスト//????//わからないから使わない

	//後々、コライダーを停止、再開する機能を追加するかも

};

