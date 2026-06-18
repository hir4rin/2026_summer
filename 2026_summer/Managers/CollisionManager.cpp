#include "CollisionManager.h"
#include "../Character/Collider.h"
#include <algorithm>
#include <cassert>

void CollisionManager::RegisterCollider(Collider* collider)
{
	//すでに登録されているかどうかを確認する
	auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);

	if (it == m_colliders.end())
	{
		m_colliders.push_back(collider);
	}
	else
	{
		assert(false && "RegisterColliderが既に登録されています");
	}

}

void CollisionManager::ReleaseCollider(Collider* collider)
{
	//登録されているものを探して、削除する
	auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);

	if (it != m_colliders.end())
	{
		m_colliders.erase(it);
	}
	else
	{
		assert(false && "RemoveCOLLIDERが見つかりませんでした");
	}
}

void CollisionManager::Clear()
{
	//???
	//生ポインタを使っているが、この方法で大丈夫か

	m_colliders.clear();
	//m_callbacks.clear();//????
}

void CollisionManager::Update()
{
	//IsTriggerを作って、押し戻し判定を無視するという条件式を追加する//今は押し戻し判定を無視する条件式はない
	//PushBackの中でその処理をするのでいい

	//この中身は未熟なので、すべて書き直す


	//すべてのコライダーの組み合わせをチェックする
	for (size_t i = 0; i < m_colliders.size(); i++)
	{
		for (size_t j = i + 1; j < m_colliders.size(); j++)
		{
			Collider* colliderA = m_colliders[i];
			Collider* colliderB = m_colliders[j];
			//アクティブなコライダーだけをチェックする//ここ関数化
			if (!colliderA|| !colliderB|| !colliderA->IsActive() || !colliderB->IsActive())
			{
				continue;
			}

			//衝突判定
			if (colliderA->IsCollible(*colliderB))
			{
				//衝突したときの処理を呼び出す
				colliderA->OnCollision(*colliderB);
				colliderB->OnCollision(*colliderA);
			
				//ここで押し戻し
			}
		}
	}
	//ここで位置確定用の関数を読んで位置をおいておく
	//ここですべてのコライダーの位置を更新させる関数
	//速度をSetVelだと、どこからでもいじれちゃうけど、CollsionManagerがColのfriendクラスになって速度をいじれるようにして、更新させる
}


void CollisionManager::DebugDraw() const
{
	//登録されているすべてのコライダーのデバッグ描画を呼び出す
	for (const auto& collider : m_colliders)
	{
		//アクティブなコライダーだけを描画する
		if (collider != nullptr && collider->IsActive())
		collider->DebugDraw();
	}
}

