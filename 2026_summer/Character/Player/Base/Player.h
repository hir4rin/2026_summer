#pragma once
#include "../../CharacterBase.h"
#include "PlayerEnums.h"
#include "PlayerStateIdle.h"//他のStateから他のStateに遷移するため(便利)
#include "PlayerStateMove.h"//以下同文
#include "PlayerStateRun.h"//以下同文
#include "PlayerStateJump.h"//以下同文
#include "PlayerStateFall.h"//以下同文
#include "PlayerStateAvoid.h"//以下同文
#include "PlayerStateAttack.h"//以下同文
#include "PlayerStateSkillAttack.h"//以下同文
#include "PlayerStateDashAttack.h"//以下同文
#include "../../../DataLoader/PlayerAnimData.h"
#include <memory>

class PlayerState;
class Camera;
class Weapon;

struct ComboNode
{
	std::string animName;//アニメーションの名
	AttackType type;//攻撃するタイプ
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
	//int nextComboIndex = -1;//次のコンボの段数
};

//いずれ行と列のなまえにしたさがある
enum ComboNodeType : int
{
	None = 0,
	AnimName = 1,
	Type = 2,
	Index = 3,
	AttackPower = 4,
	MoveTimeRate = 5,
	MoveSpeedX = 6,
	MoveSpeedY = 7,
	NextLightAttack = 8,
	NextHeavyAttack = 9,
	knockBackXZ = 10,
	knockBackY = 11,
	IsKirimomi = 12,
	SeFrameRate = 13,
	SeName = 14,
	Size = 15,
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
};

class Player : public CharacterBase, public std::enable_shared_from_this<Player>//Playerクラスのインスタンスから、Playerクラスのshared_ptrを取得できるようになる
{
public:
	Player();
	virtual ~Player();

	void Init();

	void Update(Camera& camera);
	void Draw();
	void EffectDraw();

	void OnCollision(Collider& other) override;
	void OnDamage(Collider& other, AttackData& data) override;
	ComboInfo& GetComboInfo() { return m_comboInfo; }//攻撃コンボの情報を取得する
	/// <summary>
	/// 状態を変更する関数
	/// </summary>
	/// <param name="newState"></param>
	void ChangeState(std::shared_ptr<PlayerState> newState);//状態遷移の関数//
private:
	void InitializeComboChain();//CSVからコンボデータの読み込みをする
	void UpdateAngle();//回転処理
	bool IsAvoidable()const;//回避入力を受け付けるかどうか
	void WingUpdate();//鴉状態の羽の更新
	void ApplyPos()override;//座標の適用//Playerクラスでは、座標に加えて、首のボーンの回転も適用する
private:
	//コンボチェーン
	std::vector<ComboNode> m_comboChain = {};//コンボのデータ
	ComboInfo m_comboInfo = {};//コンボの情報//現在のコンボの段数などを管理するためのもの
	AvoidInfo m_avoidInfo = {};//回避の情報

	std::shared_ptr<PlayerState> m_currentState;//プレイヤーの状態//攻撃中、移動中など//状態遷移の管理をするためのもの

	bool m_isRaven = false;//鴉状態かどうか//攻撃が変化する
	const int kPlayerNeckBoneIndex = 25;//首のボーンのインデックス
	int m_wingModelHandle = -1;//鴉の羽のモデルのハンドル//鴉状態の時に表示する
	std::shared_ptr<Weapon> m_weapon;//武器

	int m_efHandle = -1;//エフェクトのハンドル
	int m_efPlayingHandle = -1;//再生中のエフェクトのハンドル

	friend class PlayerState;//PlayerStateクラスから、Playerクラスのprivateメンバにアクセスできるようにする
	friend class PlayerStateIdle;
	friend class PlayerStateMove;
	friend class PlayerStateRun;
	friend class PlayerStateJump;
	friend class PlayerStateFall;
	friend class PlayerStateAvoid;
	friend class PlayerStateAttack;
	friend class PlayerStateSkillAttack;
	friend class PlayerStateDashAttack;

};



