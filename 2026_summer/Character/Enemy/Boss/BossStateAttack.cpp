#include "BossStateAttack.h"
#include "BossEnemy.h"
#include "BossStateBack.h"
#include "../../../Game.h"
#include "../../AttackCol.h"
#include "../../Collider.h"
#include "../../FireToarch.h"
#include "../Managers/CollisionManager.h"
#include "../EnemyManager.h"
#include "Player.h"
#include "../../../System.h"
#include "EffekseerForDXLib.h"
#include <memory>

namespace
{
	const std::string kAttack = "Player|Attack";

	constexpr float kEnemyAttackCoolTime = 120.0f;//敵の攻撃のクールタイム

	constexpr float kAttackMoveStartRate = 0.3f;//攻撃モーション:移動を開始するrate
	constexpr float kAttackColActivateRate = 0.5f;//攻撃モーション:攻撃判定を有効にするrateの上限
	constexpr float kAttackMoveSpeed1 = 10.0f;//攻撃モーション前半の移動速度
	constexpr float kAttackMoveSpeed2 = 16.0f;//攻撃モーション後半の移動速度

	constexpr float kAttackHitStopTime = 0.1f;//攻撃が当たったときのヒットストップの時間
	constexpr float kAttackColOffsetDistance = 30.0f;//攻撃判定を前に出す距離
	constexpr int kAttackTypeCount = 3;//抽選する攻撃タイプの数(Melee/Langed/Summon)

	constexpr float kMeleeAttackPower = 20.0f;//近接攻撃の攻撃力
	constexpr float kMeleeAttackForwardOffset = 100.0f;//近接攻撃判定を前方に出すオフセット距離
	const Vector3 kMeleeAttackColHeightOffset = Vector3(0, 220, 0);//近接攻撃判定の高さオフセット
	constexpr float kMeleeAttackColRadius = 100.0f;//近接攻撃判定の半径
	constexpr float kHadouEffectYOffset = 200.0f;//はどうエフェクトの高さオフセット

	constexpr float kLangedAttackPower = 15.0f;//遠隔攻撃の攻撃力
	constexpr int kFireToarchSpawnOffsetRange = 1000;//炎の生成位置のばらつき範囲(全幅)
	constexpr int kFireToarchSpawnOffsetHalf = 500;//炎の生成位置のばらつき範囲の半分
	constexpr float kFireToarchRadius = 25.0f;//炎の当たり判定の半径
	const Vector3 kFireToarchBoxHalfExtent = Vector3(60, 200, 60);//炎の当たり判定のBOXの半径
	constexpr float kFireToarchLifeTime = 250.0f;//炎の当たり判定の生存時間

	constexpr float kSummonAnimRate = 0.5f;//召喚攻撃:敵を出現させるrate
	constexpr float kSummonSpawnRange = 300.0f;//召喚攻撃:ボスからの出現範囲(半径)
	constexpr int kSummonEnemyNum = 2;//召喚攻撃で出現させる敵の数

	//ボスの周囲のランダムな位置を返す(召喚攻撃の出現位置用)
	Vector3 GetRandomSummonPos(const Vector3& center)
	{
		float x = static_cast<float>(rand() % static_cast<int>(kSummonSpawnRange * 2)) - kSummonSpawnRange;
		float z = static_cast<float>(rand() % static_cast<int>(kSummonSpawnRange * 2)) - kSummonSpawnRange;
		return center + Vector3(x, 0.0f, z);
	}
}

BossStateAttack::BossStateAttack(std::weak_ptr<BossEnemy> boss) :
	BossState(boss)
{
	m_summonEffectHandle = System::GetInstance().GetHandle(AsyncData::BossSummonEffect);
}

BossStateAttack::~BossStateAttack()
{
}

void BossStateAttack::Enter()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	//攻撃のタイプを決める
	DetermineAttackType();


	//Playerを見る
	boss->ToPlayerLook();

	//当たり判定の生成
	switch (boss->m_attackType)
	{
	case BossEnemy::AttackType::Melee:
	{
		boss->m_anim.ChangeAnim(boss->GetAnimName("Hadou"), false, 0.7f);
		AttackData data;
		data = {
			.attackPower = kMeleeAttackPower,
			.knockBackPower = Vector3(0, 0,0),
			//.knockBackPower = Vector3(0.0f,node.knockBackY,0.0f),//吹き飛ばない攻撃にする
			.knockBackFrame = 0,
			.hitStopTime = kAttackHitStopTime,
			.kAttackColOffset = kAttackColOffsetDistance,
			.isKirimomi = false
		};
		auto MeleeCol = std::make_shared<AttackCol>(m_owner,data);
		Vector3 offset = boss->m_targetVec * kMeleeAttackForwardOffset;

		MeleeCol->ColInit(boss->m_pos, kMeleeAttackColHeightOffset+offset, kMeleeAttackColRadius, ColliderType::Sphere, Tags::EnemyAttack, false, true);
		boss->m_attackCol = MeleeCol;

		//はどう攻撃のエフェクトを再生する
		m_hadouEffectHandle = System::GetInstance().GetHandle(AsyncData::BossAttackHadouEffect);
		m_hadouPlayingHandle = PlayEffekseer3DEffect(m_hadouEffectHandle);
		SetPosPlayingEffekseer3DEffect(m_hadouPlayingHandle, boss->m_pos.x, boss->m_pos.y+kHadouEffectYOffset, boss->m_pos.z);
	}
		break;
	case BossEnemy::AttackType::Langed:
	{
		boss->m_anim.ChangeAnim(boss->GetAnimName("LangedAttack"), false, 0.6f);
		//複数個fireToarchを生成
		AttackData data;
		data = {
			.attackPower = kLangedAttackPower,
			.knockBackPower = Vector3(0, 0,0),
			//.knockBackPower = Vector3(0.0f,node.knockBackY,0.0f),//吹き飛ばない攻撃にする
			.knockBackFrame = 0,
			.hitStopTime = kAttackHitStopTime,
			.kAttackColOffset = kAttackColOffsetDistance,
			.isKirimomi = false
		};
		auto player = boss->m_enemyManager.lock()->GetPlayer().lock();

		for (int i = 0; i < 10; i++)
		{
			float x = rand() % kFireToarchSpawnOffsetRange - kFireToarchSpawnOffsetHalf;
			float z = rand() % kFireToarchSpawnOffsetRange - kFireToarchSpawnOffsetHalf;


			auto fireToarchCol = std::make_shared<FireToarch>(m_owner, data);
			fireToarchCol->ColInit(Vector3(player->GetPos().x + x, 0, player->GetPos().z + z), Vector3(), kFireToarchRadius, ColliderType::Box, Tags::EnemyAttack, false, true, kFireToarchLifeTime);
			fireToarchCol->SetBoxHalfExtents(kFireToarchBoxHalfExtent);
			fireToarchCol->SetID();
			boss->m_attackCols.push_back(fireToarchCol);
		}
	}
		break;
	case BossEnemy::AttackType::Summon:
		boss->m_anim.ChangeAnim(boss->GetAnimName("LangedAttack"), false, 0.9f);
		break;
	default:
		break;
	}
}

void BossStateAttack::Update()
{
	//ここですること
	//playerとの距離によって、攻撃方法を変える
	//近くならMelee、遠くならLanged、召喚攻撃の三つの確率を変える
	//絶え間なくj攻撃して、スタン値がたまったらスタン状態になるようにする
	//まず、めれー攻撃をする
	//auto attackCol = std::make_shared<AttackCol>();




	auto boss = m_owner.lock();
	if (!boss)return;

	switch (boss->m_attackType)
	{
	case BossEnemy::AttackType::Melee:
		//当たり判定を途中からON
		if (boss->m_anim.GetAnimRate() > 0.1f)
		{
			boss->m_attackCol->SetIsActive(true);
		}
		break;
	case BossEnemy::AttackType::Langed:
		break;
	case BossEnemy::AttackType::Summon:
		if (!isSpawned && boss->m_anim.GetAnimRate() > kSummonAnimRate)
		{
			isSpawned = true;

			//ランダムな位置に敵を2体出現させる
			auto enemyManager = boss->m_enemyManager.lock();
			if (enemyManager)
			{
				for (int i = 0; i < kSummonEnemyNum; i++)
				{
					Vector3 summonPos = GetRandomSummonPos(boss->m_pos);
					enemyManager->SpawnEnemy(summonPos);

					//召喚位置にエフェクトを再生する
					int playingHandle = PlayEffekseer3DEffect(m_summonEffectHandle);
					SetPosPlayingEffekseer3DEffect(playingHandle, summonPos.x, summonPos.y, summonPos.z);
				}
			}
		}
		break;
	default:
		break;
	}


	if (boss->m_anim.GetAnimEndFlag())
	{
		boss->m_attackCoolTime = kEnemyAttackCoolTime;//攻撃のクールタイムをリセット
		boss->ChangeState(std::make_shared<BossStateIdle>(m_owner));
		return;
	}
}

void BossStateAttack::Exit()
{
	auto boss = m_owner.lock();
	if (!boss)return;
	boss->m_vel = Vector3(0, 0, 0);
	boss->m_attackCol->SetIsActive(false);
	boss->m_attackCol->ClearHitIds();
	isSpawned = false;

	//はどう攻撃のエフェクトを止める
	if (m_hadouPlayingHandle != -1)
	{
		StopEffekseer3DEffect(m_hadouPlayingHandle);
		m_hadouPlayingHandle = -1;
	}

	//BossEnemy::AttackType::Langed
}

void BossStateAttack::DebugDraw()
{
	DrawFormatString(Game::kScreenWidth - 600, 10, GetColor(255, 255, 255), "Boss State: Attack");
}

void BossStateAttack::DetermineAttackType()
{
	//攻撃の種類を決める
	auto boss = m_owner.lock();
	if (!boss)return;
	//確率で決める
	int rate = rand() % kAttackTypeCount;

	switch (rate)
	{
	case 0:
		boss->m_attackType = BossEnemy::AttackType::Melee;
		break;
	case 1:
		boss->m_attackType = BossEnemy::AttackType::Langed;
		break;
	case 2:
		boss->m_attackType = BossEnemy::AttackType::Summon;
		break;

	}
		
	//boss->m_attackType = BossEnemy::AttackType::Langed;


	//攻撃判定を生成

}

void BossStateAttack::MeleeAttack()
{
	auto boss = m_owner.lock();
	if (!boss)return;

	float rate = boss->m_anim.GetAnimRate();
	Vector3 forward = boss->m_targetPos - boss->m_pos;
	forward.y = 0.0f;
	forward = forward.Normalize();
	//移動距離
	if (rate <= kAttackMoveStartRate)
	{
		boss->m_vel = forward * kAttackMoveSpeed1;
	}
	else if (rate > kAttackMoveStartRate && rate <= kAttackColActivateRate)
	{
		boss->m_vel = forward * kAttackMoveSpeed2;
		boss->m_attackCol->SetIsActive(true);
	}
	else
	{
		boss->m_vel = Vector3(0, 0, 0);
	}
}
