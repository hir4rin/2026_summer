#include "AttackCol.h"
#include "CharacterBase.h"
#include "Player.h"
#include "../Camera/CameraManager.h"
#include "../Camera/LockOnManager.h"
#include "../Camera/MainCamera.h"
#include "../System.h"
#include "Enemy/EnemyBase.h"
#include "../Effect/EffectManager.h"
#include "EffekseerForDXLib.h"

namespace
{
	constexpr float kAttackColOffset = 50.0f;//攻撃判定を前に出す距離//本来はここも攻撃ごとに変えるべき

	constexpr float kCameraShakePower = 2.5f;//カメラの揺れの強さ
	constexpr float kCameraShakeTime = 5.0f;//カメラの揺れの時間

	constexpr float kEfOffset = 80.0f;//エフェクトの座標のオフセット
}

AttackCol::AttackCol(std::weak_ptr<CharacterBase> owner,const AttackData& data)
	: m_owner(owner)
{
	if (m_owner.expired())return;
	//AttackDataを保持
	m_attackData = std::make_shared<AttackData>(data);
	//通常エフェクトを出す
	m_hitEfHandle = System::GetInstance().GetHandle(AsyncData::PlayerHitEffect);
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
		case Tags::PlayerUltAttack:
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
void AttackCol::Update()
{

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
			// 初めての敵と当たった場合の処理

			//プレイヤーのゲージ管理//今は複数の敵に当たったらその分ゲージが上がるようになっている
			PlayerGaugeUp(other);
			//リザルト集計用//与えたダメージ、コンボ数(総ヒット数)を加算する
			player->AddAttackResult(m_attackData->attackPower);

			//otherの被ダメ処理
			
			//hitColのOnDamageInterFaceを呼ぶ
			auto hitCol = dynamic_cast<HitCol*>(&other);
			if (hitCol)
			{
				auto cameraManager = player->GetCameraManager().lock();
				auto lockOnManager = player->GetLockOnManager().lock();
				if (!cameraManager)
				{
					m_hitIds.push_back(otherId);//当たったidのリストにotherのidを追加する
					return;
				}
				auto mainCamera = cameraManager->GetMainCamera();
				if (!mainCamera)
				{
					m_hitIds.push_back(otherId);//当たったidのリストにotherのidを追加する
					return;
				}

				///---------
				/// ここで、Playerの初めて当たった時という関数を呼び出して、
				/// そこでカメラを揺らしたり、ターゲットを保存したりする
				///---------

				//最初にあたった攻撃だったらカメラを揺らす
				if(m_hitIds.empty())mainCamera->StartCameraShake(kCameraShakePower, kCameraShakeTime);//カメラを揺らす

				//attackDataの変更//現在経過時間を引いて、敵の移動距離、時間を決める
				float nowAnimFrame = player->GetAnimation().GetNowAnimFrame();
				m_attackData->knockBackFrame -= nowAnimFrame;
				//ダメージの受け渡し
				hitCol->OnDamageInterFace(*this, *m_attackData);
				//ヒットストップの受け渡し
				//hitCol->SetTimeScaleInterFace(0.3f, 10.0f);

				//プレイヤーの攻撃が当たった時の処理//カメラシェイクや、内部ターゲットのセット
				player->OnAttackHit(otherId);


				//ownerに当たったことを連絡->AttackMoveを止める
				bool isLockOn = mainCamera->GetIsLockOn();
				if (isLockOn)
				{
					//ロックオンしている敵がいて、そいつに当たったら攻撃の移動を止める
					auto playerTarget = player->GetTargetEnemy();
					auto targetEnemy = playerTarget.lock();
					if (!targetEnemy)
					{
						assert(false && "PlayerAttackOnCollision:ターゲットしている敵がいません");
					}
					if (otherId == targetEnemy->GetId())
					{
						//攻撃の移動を止める
						auto& comboInfo = player->GetComboInfo();
						comboInfo.isHit = true;//攻撃が当たったことを通知する//これで、攻撃の移動を止める
					}
				}
				//ロックオンしていない場合
				else
				{
					//内部ターゲットにセットする//最初の敵だったら
					if (m_hitIds.empty())lockOnManager->SetTargetEnemy(otherId);

				}
				
			}
			//もしプレイヤーの必殺技攻撃だったら
			if (GetTag() == Tags::PlayerUltAttack)
			{
				//演出が始まっていなかったら
				bool isUltStart = System::GetInstance().GetIsUltimating();
				if (!isUltStart)
				{
					System::GetInstance().SetUltStart(120);//必殺技の演出をスタートする
					System::GetInstance().SetTimeScaleForFrames(0.1f, 120);//時間を遅くする//60フレームで元に戻す
				}
				//必殺技の被ダメエフェクト
				m_hitEfPlayingHandle = EffectManager::GetInstance().Play(AsyncData::EnemyHitEffectUlt,
					Vector3(other.GetPos().x, other.GetPos().y + kEfOffset*1.7f, other.GetPos().z),0.0f,0.9f);
			}
			//もしプレイヤーの通常攻撃だったら
			else
			{
				/*m_hitEfPlayingHandle = PlayEffekseer3DEffect(m_hitEfHandle);
				SetPosPlayingEffekseer3DEffect(m_hitEfPlayingHandle, other.GetPos().x, other.GetPos().y+ kEfOffset, other.GetPos().z);*/
				m_hitEfPlayingHandle = EffectManager::GetInstance().Play(AsyncData::EnemyHitEffect,
					Vector3(other.GetPos().x, other.GetPos().y + kEfOffset, other.GetPos().z));
			}
			m_hitIds.push_back(otherId);//当たったidのリストにotherのidを追加する
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

void AttackCol::PlayerGaugeUp(Collider& other)
{
	auto it = m_owner.lock();
	if (!it)return;
	auto player = std::dynamic_pointer_cast<Player>(it);
	if (!player)return;


	//通常攻撃ならスキル攻撃をあげる
	if (!player->GetIsRaven())
	{
		//スキルゲージの上昇
		player->AddSkillGauge(20);
		//必殺技ゲージの上昇
		player->AddUltGauge(10);
	}
	//raven状態なら
	else
	{
		//必殺技ではないのならスキルゲージを上げる//スキル攻撃
		if (GetTag() != Tags::PlayerUltAttack)
		{
			//必殺技ゲージの上昇
			player->AddUltGauge(20);
		}

	}

	
}
