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
	//すべてのコライダーの組み合わせをチェックする
	for (size_t i = 0; i < m_colliders.size(); i++)
	{
		for (size_t j = i + 1; j < m_colliders.size(); j++)
		{
			Collider* colliderA = m_colliders[i];
			Collider* colliderB = m_colliders[j];
			//アクティブなコライダーだけをチェックする
			if (colliderA == nullptr || colliderB == nullptr || !colliderA->isActive() || !colliderB->isActive())
			{
				continue;
			}
			//衝突判定
			if (colliderA->IsCollible(*colliderB))
			{
				//衝突したときの処理を呼び出す
				colliderA->OnCollision(*colliderB);
				colliderB->OnCollision(*colliderA);
			
			}
		}
	}
}


void CollisionManager::DebugDraw() const
{
	//登録されているすべてのコライダーのデバッグ描画を呼び出す
	for (const auto& collider : m_colliders)
	{
		//アクティブなコライダーだけを描画する
		if (collider != nullptr && collider->isActive())
		collider->DebugDraw();
	}
}

