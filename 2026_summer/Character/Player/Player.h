#pragma once
#include "../CharacterBase.h"
#include "PlayerStateIdle.h"
#include "PlayerStateMove.h"
#include "../../DataLoader/PlayerAnimData.h"
#include <memory>

enum class  AttackType
{
	lightAttack = 0,
	heavyAttack = 1
};

struct ComboNode
{
	std::string animName;//アニメーションの名
	AttackType type;//攻撃するタイプ
	float moveFrame=-1;//突進する時間
	float moveSpeed = -1;//突進する速度
	std::vector<int> nextWeakAttack;//弱攻撃ボタンでつながる次のコンボ番号
	std::vector<int> nextHeavyAttack;//強攻撃ボタンでつながる次のコンボ番号
	float seFrameRate = -1;//攻撃のSEを鳴らすフレームの割合//アニメーションの再生時間に対する割合で指定
	std::string seName;//攻撃のSEの名前
	bool isEffectActive = false;//強攻撃の最終段用	
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
private:

	//コンボチェーン
	std::vector<ComboNode> m_comboChain = {};//コンボのデータ
	
	
	
	std::shared_ptr<PlayerState> m_currentState;//プレイヤーの状態//攻撃中、移動中など//状態遷移の管理をするためのもの

	friend class PlayerState;//PlayerStateクラスから、Playerクラスのprivateメンバにアクセスできるようにする
	friend class PlayerStateIdle;
	friend class PlayerStateMove;

};



