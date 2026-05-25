#pragma once
#include "../CharacterBase.h"
#include "PlayerEnums.h"
#include "PlayerStateIdle.h"//他のStateから他のStateに遷移するため(便利)
#include "PlayerStateMove.h"//以下同文
#include "PlayerStateAvoid.h"//以下同文
#include "PlayerStateParry.h"//以下同文
#include "PlayerStateAttack.h"//以下同文
#include "../../DataLoader/PlayerAnimData.h"
#include <memory>



struct ComboNode
{
	std::string animName;//アニメーションの名
	AttackType type;//攻撃するタイプ
	float moveFrame = -1;//突進する時間
	float moveSpeed = -1;//突進する速度
	std::vector<int> nextWeakAttack;//弱攻撃ボタンでつながる次のコンボ番号
	std::vector<int> nextHeavyAttack;//強攻撃ボタンでつながる次のコンボ番号
	float seFrameRate = -1;//攻撃のSEを鳴らすフレームの割合//アニメーションの再生時間に対する割合で指定
	std::string seName;//攻撃のSEの名前
	bool isEffectActive = false;//強攻撃の最終段用	
};
struct ComboInfo
{
	int currentComboIndex = -1;//現在のコンボの段数//攻撃の段数を管理するための変数
	//int nextComboIndex = -1;//次のコンボの段数
};
enum ComboNodeType : int
{
	None = -1,
	AnimName = 0,
	Type = 1,
	MoveFrame = 2,
	MoveSpeed = 3,
	NextWeakAttack = 4,
	NextHeavyAttack = 5,
	SeFrameRate = 6,
	SeName = 7,
	IsEffectActive = 8
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
	constexpr int HeavyAttack3 = 7;
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

class PlayerState;
class Camera;

class Player : public CharacterBase, public std::enable_shared_from_this<Player>//Playerクラスのインスタンスから、Playerクラスのshared_ptrを取得できるようになる
{
public:
	Player();
	virtual ~Player();

	void Init();

	void Update(Camera& camera);
	void Draw();

	void OnCollision(Collider& other) override;

	/// <summary>
	/// 状態を変更する関数
	/// </summary>
	/// <param name="newState"></param>
	void ChangeState(std::shared_ptr<PlayerState> newState);//状態遷移の関数//
private:
	void InitializeComboChain();//CSVからコンボデータの読み込みをする
	void UpdateAngle();//回転処理
	bool IsAvoidable()const;//回避入力を受け付けるかどうか
private:

	//コンボチェーン
	std::vector<ComboNode> m_comboChain = {};//コンボのデータ
	ComboInfo m_comboInfo = {};//コンボの情報//現在のコンボの段数などを管理するためのもの
	AvoidInfo m_avoidInfo = {};//回避の情報

	std::shared_ptr<PlayerState> m_currentState;//プレイヤーの状態//攻撃中、移動中など//状態遷移の管理をするためのもの

	friend class PlayerState;//PlayerStateクラスから、Playerクラスのprivateメンバにアクセスできるようにする
	friend class PlayerStateIdle;
	friend class PlayerStateMove;
	friend class PlayerStateAvoid;
	friend class PlayerStateParry;
	friend class PlayerStateAttack;

};



