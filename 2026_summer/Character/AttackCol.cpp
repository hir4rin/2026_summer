#include "AttackCol.h"
#include "CharacterBase.h"
#include "Player.h"
#include "../System.h"

namespace
{
	constexpr float kAttackColOffset = 50.0f;//攻撃判定を前に出す距離//本来はここも攻撃ごとに変えるべき
}

AttackCol::AttackCol(std::weak_ptr<CharacterBase> owner,const AttackData& data)
	: m_owner(owner)
{
	if (m_owner.expired())return;
	//AttackDataを保持
	m_attackData = std::make_shared<AttackData>(data);
}

AttackCol::~AttackCol()
{
}

void AttackCol::OnCollision(Collider& other)
{
	//idで当たったかどうかを管理する//当たったidのリストにotherのidがないとき、攻撃を当てる
	//当たっていたらotherの被ダメ処理をして、Ownerに当たったことを通知してもよい
	auto owner = m_owner.lock();
	if (!owner) return;

	//Tag処理
	//Staticなら早期リターン
	auto tag = GetTag();
	switch (tag)
	{
		case Tags::PlayerAttack:
			//EnemyHitに当たったら処理する
			PlayerAttackOnCollision(other);
			break;
		case Tags::EnemyAttack:
			EnemyAttackOnCollision(other);
			break;
		default:
			//早期リターン
			return;
			break;
	}
	//if(other.GetTag() == Tags::PlayerHit || other.GetTag() == Tags::EnemyHit)
	//{
	//	int otherId = other.GetId();
	//	auto it = std::find(m_hitIds.begin(), m_hitIds.end(), otherId);
	//	if (it == m_hitIds.end())
	//	{
	//		// 当たっていない場合の処理
	//		//otherの被ダメ処理
	//		m_hitIds.push_back(otherId);//当たったidのリストにotherのidを追加する
	//		//hitColのOnDamageInterFaceを呼ぶ
	//		auto hitCol = dynamic_cast<HitCol*>(&other);
	//		if (hitCol)
	//		{
	//			hitCol->OnDamageInterFace(*this, *m_attackData);
	//		}
	//	}
	//}
	//else
	//{
	//	// 当たっている場合の処理
	//	return;
	//}
}

void AttackCol::ApplyPos()
{
	auto owner = m_owner.lock();
	if (!owner)return;
	//座標の更新//所有者のvelをもらって更新
	m_pos = owner->GetPos();
	//m_attackDataに基づいて、このkAttackColOffsetを変えるようにしないといけない
	m_pos += owner->GetTargetVec() * kAttackColOffset;//攻撃判定を前に出す
}

void AttackCol::PlayerAttackOnCollision(Collider& other)
{
	auto it = m_owner.lock();
	if (!it)return;
	auto player = std::dynamic_pointer_cast<Player>(it);
	if (!player)return;

	if (other.GetTag() == Tags::EnemyHit)
	{
		int otherId = other.GetId();
		auto it = std::find(m_hitIds.begin(), m_hitIds.end(), otherId);
		if (it == m_hitIds.end())
		{
			// 初めて当たった場合の処理
			//otherの被ダメ処理
			m_hitIds.push_back(otherId);//当たったidのリストにotherのidを追加する
			//hitColのOnDamageInterFaceを呼ぶ
			auto hitCol = dynamic_cast<HitCol*>(&other);
			if (hitCol)
			{
				//attackDataの変更//現在経過時間を引いて、敵の移動距離、時間を決める
				float nowAnimFrame = player->GetAnimation().GetNowAnimFrame();
				m_attackData->knockBackFrame -= nowAnimFrame;

				hitCol->OnDamageInterFace(*this, *m_attackData);
				//ownerに当たったことを連絡->AttackMoveを止める
				auto& comboInfo = player->GetComboInfo();
				comboInfo.isHit = true;//攻撃が当たったことを通知する//これで、攻撃の移動を止める
			}
			//もしプレイヤーの攻撃だったら
			auto player = std::dynamic_pointer_cast<Player>(m_owner.lock());
			if (player)
			{
				//演出が始まっていなかったら
				bool isUltStart = System::GetInstance().GetIsUltStart();
				if (!isUltStart)
				{
					System::GetInstance().SetUltStart();//必殺技の演出をスタートする
					System::GetInstance().SetTimeScaleForFrames(0.1f, 60);//時間を遅くする//60フレームで元に戻す
				}
			}

		}
	}
	else
	{
		// 当たっていた場合の処理//なにもしない
		return;
	}
}

void AttackCol::EnemyAttackOnCollision(Collider& other)
{
	if (other.GetTag() == Tags::PlayerHit)
	{
		int otherId = other.GetId();
		auto it = std::find(m_hitIds.begin(), m_hitIds.end(), otherId);
		if (it == m_hitIds.end())
		{
			// 当たっていない場合の処理
			//otherの被ダメ処理
			m_hitIds.push_back(otherId);//当たったidのリストにotherのidを追加する
			//hitColのOnDamageInterFaceを呼ぶ
			auto hitCol = dynamic_cast<HitCol*>(&other);
			if (hitCol)
			{
				hitCol->OnDamageInterFace(*this, *m_attackData);
			}
		}
	}
	else
	{
		// 当たっていた場合の処理//なにもしない
		return;
	}
}
