#pragma once
#include "../../CharacterBase.h"
#include "PlayerEnums.h"
#include "PlayerStateIdle.h"//他のStateから他のStateに遷移するため(便利)
#include "PlayerStateMove.h"//以下同文
#include "PlayerStateRun.h"//以下同文
#include "PlayerStateJump.h"//以下同文
#include "PlayerStateFall.h"//以下同文
#include "PlayerStateAvoid.h"//以下同文
#include "PlayerStateHit.h"//以下同文
#include "PlayerStateAttack.h"//以下同文
#include "PlayerStateSkillAttack.h"//以下同文
#include "PlayerStateUlt.h"//以下同文
#include "PlayerStateDie.h"//以下同文
#include "PlayerStateDashAttack.h"//以下同文
#include "../../../DataLoader/PlayerAnimData.h"
#include <memory>

class PlayerState;
class Camera;
class Weapon;
class CameraManager;
class EnemyBase;
class EnemyManager;

struct ComboNode
{
	std::string animName;//アニメーションの名
	int  modelType = 0;//モデルの種類//0:通常モデル、1:攻撃モデル
	AttackType type = AttackType::None;//攻撃するタイプ
	int index = -1;//攻撃の種類を管理するための変数
	float attackPower = 0;//攻撃力
	float moveFrame = -1;//突進する時間
	float moveSpeedX = 0;//前方向に突進する速度
	float moveSpeedY = 0;//垂直方向の速度
	std::vector<int> nextWeakAttack;//弱攻撃ボタンでつながる次のコンボ番号
	std::vector<int> nextHeavyAttack;//強攻撃ボタンでつながる次のコンボ番号
	float knockBackXZ = 0;//XZ方向のノックバックの距離//攻撃を受けたときに、どれくらいふっとぶか
	float knockBackY = 0;//Y方向のノックバックの距離//攻撃を受けたときにY軸に飛ぶ量
	bool isKirimomi = false;//吹っ飛ぶかどうか//吹っ飛ばない攻撃は、相手を引き寄せるような攻撃にする<-かなりあり！！！！！！！！
	float seFrameRate = -1;//攻撃のSEを鳴らすフレームの割合//アニメーションの再生時間に対する割合で指定
	std::string seName;//攻撃のSEの名前
};
struct ComboInfo
{
	int currentComboIndex = -1;//現在のコンボの段数//攻撃の段数を管理するための変数
	bool isHit = false;//攻撃が当たったかどうか//当たっていたら動きを止める
	bool isAirAttack = false;//空中で攻撃をしたかどうか
	bool isAirSkillAttack = false;//空中でスキル攻撃をしたかどうか
	int SkillGauge = 0;//スキルゲージ
	int UltGauge = 0;//必殺技ゲージ
	//int nextComboIndex = -1;//次のコンボの段数
};

//いずれ行と列のなまえにしたさがある
enum ComboNodeType : int
{
	None = 0,
	AnimName = 1,
	Model = 2,
	Type = 3,
	Index = 4,
	AttackPower = 5,
	MoveTimeRate = 6,
	MoveSpeedX = 7,
	MoveSpeedY = 8,
	NextLightAttack = 9,
	NextHeavyAttack = 10,
	knockBackXZ = 11,
	knockBackY = 12,
	IsKirimomi = 13,
	SeFrameRate = 14,
	SeName = 15,
	Size = 16,
};
namespace ComboIndex
{
	constexpr int None = -1;
	constexpr int LightAttack1 = 0;
	constexpr int LightAttack2 = 1;
	constexpr int LightAttack3 = 2;
	constexpr int LightAttack4 = 3;
	constexpr int LightAttack5 = 4;

	constexpr int HeavyAttack1 = 5;
	constexpr int HeavyAttack2 = 6;

	constexpr int upAttack = 7;

	constexpr int AirAttack1 = 8;
	constexpr int AirAttack2 = 9;
	constexpr int AirAttack3 = 10;
	constexpr int AirAttack4 = 11;

	constexpr int AirHeavyAttack1 = 12;

	constexpr int DashAttack = 13;
	constexpr int SkillAttack1 = 14;
	constexpr int SkillAttack2 = 15;
	constexpr int SkillAttack3 = 16;
};

struct AvoidInfo
{
	bool isAvoiding = false;//回避中かどうかのフラグ
	bool isAvoidReserved = false;//回避の予約
	Vector3 BaseVel = Vector3();//基準とするベクトルをセット
	float avoidCount = 0.0f;//回避のカウント//回避の時間を管理するためのカウント
	float avoidCoolTimeCount = 0.0f;//回避のクールタイムのカウント//回避のクールタイムを管理するためのカウント
	bool isAvoidBack = false;//後ろに回避するかどうかのフラグ	
	bool isJustAvoid = false;//ジャスト回避かどうかのフラグ
};
struct DamageInfo
{
	float damageTimer = 0.0f;//被ダメ後無敵時間
	const float kDamageTime = 180.0f;//被ダメ後無敵時間の長さ
};

enum class WaveNumForPlayer : int
{
	Wave1 = 0,
	Wave2 = 1,
	Wave3 = 2,
	WaveSize = 3,
};

class Player : public CharacterBase//Playerクラスのインスタンスから、Playerクラスのshared_ptrを取得できるようになる
{
public:
	Player();
	virtual ~Player();

	void Init();
	void SetCameraManager(std::weak_ptr<CameraManager> cameraManager) { m_cameraManager = cameraManager; }//カメラマネージャーのセット
	void SetEnemyManager(std::weak_ptr<EnemyManager> enemyManager) { m_enemyManager = enemyManager; }//EnemyManagerのセット

	void Update(Camera& camera);
	void Draw();
	void EffectDraw();

	void OnCollision(Collider& other) override;
	void OnDamage(Collider& other, AttackData& data) override;
	ComboInfo& GetComboInfo() { return m_comboInfo; }//攻撃コンボの情報を取得する

	float GetCameraRockOnRange()const { return kPlayerRockOnRange; }//ロックオンする範囲を返す

	//スキルゲージの増減
	void AddSkillGauge(int value);//スキル増減
	//必殺技ゲージの増減
	void AddUltGauge(int value);//必殺技増減
	//ゲージの取得
	int GetSkillGauge()const { return m_comboInfo.SkillGauge; }//スキルゲージの取得
	int GetUltGauge()const { return m_comboInfo.UltGauge; }//必殺技ゲージの取得

	//鴉状態かどうか
	bool GetIsRaven()const { return m_isRaven; }

	//ターゲットしている敵の取得
	std::weak_ptr<EnemyBase> GetTargetEnemy()const;
	//内部ロックオンのために
	std::weak_ptr<CameraManager> GetCameraManager()const { return m_cameraManager; }
	//プレイヤーの移動制限
	void SetLimitPlayerArea(int num,bool value) { m_isWaveArea[num] = value; }

private:
	/// <summary>
	/// 状態を変更する関数
	/// </summary>
	/// <param name="newState"></param>
	void ChangeState(std::shared_ptr<PlayerState> newState);//状態遷移の関数//

	void InitializeComboChain();//CSVからコンボデータの読み込みをする
	void UpdateAngle();//回転処理
	bool IsAvoidable()const;//回避入力を受け付けるかどうか
	void WingUpdate();//鴉状態の羽の更新
	void ApplyPos()override;//座標の適用//Playerクラスでは、座標に加えて、首のボーンの回転も適用する
	void ApplyPosWithAttackModel();//アタックモデルにも適用

	bool CanSkillAttack(bool changeGauge = true);//スキル攻撃ができるかどうか//trueならゲージを減らす
	bool CanUltAttack();//必殺技攻撃ができるかどうか//trueならゲージを減らす

	std::shared_ptr<Player> GetSharedPtr() {return std::dynamic_pointer_cast<Player>(shared_from_this());}

	std::weak_ptr<Player> GetWeakPtr() {return GetSharedPtr();}

private:
	//コンボチェーン
	std::vector<ComboNode> m_comboChain = {};//コンボのデータ
	ComboInfo m_comboInfo = {};//コンボの情報//現在のコンボの段数などを管理するためのもの
	AvoidInfo m_avoidInfo = {};//回避の情報
	DamageInfo m_damageInfo = {};//被ダメ後無敵時間の情報

	bool m_isWaveArea[static_cast<int>(WaveNumForPlayer::WaveSize)] = {false};//ウェーブごとに敵がスポーンしたかどうかのフラグ//EnemyManagerのフラグと同じものを持つ

	std::shared_ptr<PlayerState> m_currentState;//プレイヤーの状態//攻撃中、移動中など//状態遷移の管理をするためのもの
	std::shared_ptr<PlayerState> m_prevState;//前の状態

	bool m_isRaven = false;//鴉状態かどうか//攻撃が変化する
	const int kPlayerNeckBoneIndex = 25;//首のボーンのインデックス
	int m_wingModelHandle = -1;//鴉の羽のモデルのハンドル//鴉状態の時に表示する
	std::shared_ptr<Weapon> m_weapon;//武器

	int m_attackModelHandle = -1;//攻撃のモデルのハンドル

	int m_efHandle = -1;//エフェクトのハンドル
	int m_efPlayingHandle = -1;//再生中のエフェクトのハンドル

	const float kPlayerRockOnRange = 300.0f;//ロックオンする範囲//この範囲内に敵がいたらロックオンする

	const float kJustAvoidRadius = 500.0f;//ジャスト回避の範囲

	std::weak_ptr<CameraManager> m_cameraManager;//カメラマネージャ-の弱参照
	std::weak_ptr<EnemyManager> m_enemyManager;//EnemyManagerの弱参照
	

	//PlayerState
	friend class PlayerState;//PlayerStateクラスから、Playerクラスのprivateメンバにアクセスできるようにする
	friend class PlayerStateIdle;
	friend class PlayerStateMove;
	friend class PlayerStateRun;
	friend class PlayerStateJump;
	friend class PlayerStateFall;
	friend class PlayerStateAvoid;
	friend class PlayerStateHit;
	friend class PlayerStateAttack;
	friend class PlayerStateSkillAttack;
	friend class PlayerStateUlt;
	friend class PlayerStateDie;
	friend class PlayerStateDashAttack;
	//武器
	friend class Weapon;

};



