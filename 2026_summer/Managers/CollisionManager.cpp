#include "CollisionManager.h"
#include "../Character/Collider.h"
#include"../System.h"
#include <algorithm>
#include <cassert>

void CollisionManager::RegisterCollider(std::shared_ptr<Collider> collider)
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

void CollisionManager::ReleaseCollider(std::shared_ptr<Collider> collider)
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

void CollisionManager::Init()
{
	m_collisionChecker = std::make_unique<CollisionChecker>();
	m_fixNextPositioner = std::make_unique<FixNextPosition>();
}

void CollisionManager::Terminate()
{
	m_colliders.clear();
}

void CollisionManager::Update()
{
	//IsTriggerを作って、押し戻し判定を無視するという条件式を追加する//今は押し戻し判定を無視する条件式はない
	//PushBackの中でその処理をするのでいい

	//CollisionのUpdate(今は寿命カウント用)
	for(auto& collider  : m_colliders)
	{
		if (!collider)continue;
		collider->ColUpdate();
	}


	//寿命が尽きたコライダーを削除する
	std::erase_if(m_colliders, [](const std::shared_ptr<Collider>& collider) 
		{
			return collider->GetIsLifeTimeLimited();
		});


	//すべてのコライダーの組み合わせをチェックする//当たっているかの確認かつ、速度をいじる
	for (size_t i = 0; i < m_colliders.size(); i++)
	{
		std::shared_ptr<Collider> colliderA = m_colliders[i];
		if (!colliderA->IsActive())continue;
		//if (colliderA->GetTag() == Tags::StaticObject)continue;//静的オブジェクトがAの時無視

		for (size_t j = i + 1; j < m_colliders.size(); j++)
		{
			
			std::shared_ptr<Collider> colliderB = m_colliders[j];
			//アクティブなコライダーだけをチェックする//ここ関数化
			if (!colliderB)continue;
			if (!colliderB->IsActive())continue;
			//静的オブジェクト同士の時無視
			if (colliderA->GetTag() == Tags::StaticObject &&
				colliderB->GetTag() == Tags::StaticObject)continue;
			//それぞれの速度の更新
			float timescale = System::GetInstance().GetTimeScale();
			//ここですべての速度にtimescaleをかける
			colliderA->m_vel *= timescale * colliderA->GetTimeScale();
			colliderB->m_vel *= timescale * colliderB->GetTimeScale();
			//衝突判定//球と球、BoxとBox、CapsuleとCapsuleとかで分ける
			if (m_collisionChecker->IsCollide(*colliderA, *colliderB))
			{

				//衝突したときの処理を呼び出す
				colliderA->OnCollision(*colliderB);
				colliderB->OnCollision(*colliderA);

				//ここで押し戻し
				//isTriggerは押し戻しを無視
				if (colliderA->GetIsTrigger() || colliderB->GetIsTrigger()) continue;
				//押し戻しの処理
				//ここで速度を変更する//ここでタイムスケールを変更<-？？多分違う
				//PushBackのvelを加える
				m_fixNextPositioner->FixNextPos(*colliderA, *colliderB);
			}
			////球と球
			//if (colliderA->IsCollidable(*colliderB))
			//{

			//	//衝突したときの処理を呼び出す
			//	colliderA->OnCollision(*colliderB);
			//	colliderB->OnCollision(*colliderA);
			//
			//	//ここで押し戻し
			//	//isTriggerは押し戻しを無視
			//	if(colliderA->GetIsTrigger() || colliderB->GetIsTrigger()) continue;
			//	//押し戻しの処理
			//	//ここで速度を変更する//ここでタイムスケールを変更<-？？多分違う
			//	//PushBackのvelを加える
			//	colliderA->m_vel += colliderA->PushBack(*colliderB);
			//	colliderB->m_vel += colliderB->PushBack(*colliderA);
			//}
		}
	}
	//ここで位置確定用の関数を読んで位置をおいておく
	//ここですべてのコライダーの位置を更新させる関数
	//速度をSetVelだと、どこからでもいじれちゃうけど、CollisionManagerがColのfriendクラスになって速度をいじれるようにして、更新させる
	//速度を足す
	ApplyAdjustments();
}


void CollisionManager::DebugDraw() const
{
	//登録されているすべてのコライダーのデバッグ描画を呼び出す
	for (const auto& collider : m_colliders)
	{
		//アクティブなコライダーだけを描画する
		if (collider && collider->IsActive())
		collider->DebugDraw();
	}
}

void CollisionManager::ApplyAdjustments()
{
	//Colliderの座標を確定//Col自身に座標の更新をさせる
	for (auto& collider : m_colliders)
	{
		if (!collider)continue;
		collider->ApplyPos();
	}
}

