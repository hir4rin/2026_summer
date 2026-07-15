#pragma once

//ゲーム全体で使用する定数
namespace Game
{
	//画面情報
	constexpr int kScreenWidth = 1280;
	constexpr int kScreenHeight = 720;
	constexpr int kColorBitNum = 32;

	//重力
	constexpr float kGravity = 1.0f;//重力の加速度//この数値を変えることで、ジャンプの高さや落ちる速さを調整できる

	//Playerの移動スピード
	constexpr float kMoveSpeed = 8.0f;//移動速度
	constexpr float kAirMaxSpeed = 5.5f;//空中での最大移動速度


	//敵の移動スピード
	constexpr float kEnemyMoveSpeed = 4.0f;//敵の移動速度
	constexpr float kEnemyBackSpeed = 5.0f;//敵の距離を取るときの移動速度
}



//UIの表示座標一覧
namespace UI
{
	constexpr int kBarScale = 300;//バーの長さ//割合

	/*constexpr int HpBarX = Game::kScreenWidth * 1 / 2;
	constexpr int HpBarY = Game::kScreenHeight / 40;*/
	constexpr int HpBarX = 10;
	constexpr int HpBarY = 20;

	/*constexpr int SkillGaugeBarX = Game::kScreenWidth / 15;
	constexpr int SkillGaugeBarY = Game::kScreenHeight / 20;*/
	constexpr int SkillGaugeBarX = 10;
	constexpr int SkillGaugeBarY = 90;

	/*constexpr int UltGaugeBarX = Game::kScreenWidth / 15;
	constexpr int UltGaugeBarY = Game::kScreenHeight / 10;*/
	constexpr int UltGaugeBarX = 10;
	constexpr int UltGaugeBarY = 160;
}

enum class ComboChainButton : int
{
	X = 0,
	Y = 1,
	A = 2,
	B = 3,
	RB = 4,	
	RT = 5,
	LB = 6,
	LT = 7,
	RStickLR = 8,
	ButtonSize = 9
};

namespace ComboUI
{
	constexpr float kButtonScale = 0.3f;//ボタンの大きさ//倍率

	constexpr int ButtonToButtonDistanceX = 50;//ボタンとボタンの間の距離//横
	constexpr int ButtonToButtonDistanceY = 60;//ボタンとボタンの間の距離//縦

	constexpr int kStringDistanceY = 35;//文字とボタンの間の距離//縦


	//弱5ボタン
	constexpr ComboChainButton kCombo1[] = { ComboChainButton::X, ComboChainButton::X, ComboChainButton::X,ComboChainButton::X,ComboChainButton::X };
	//開始位置
	const int kCombo1StartX = Game::kScreenWidth* 8 /10;
	const int kCombo1StartY = 100 + ButtonToButtonDistanceY * 1;


	//強2ボタン
	constexpr ComboChainButton kCombo2[] = { ComboChainButton::Y, ComboChainButton::Y };
	//開始位置
	const int kCombo2StartX = Game::kScreenWidth *8 / 10;
	const int kCombo2StartY = 100+ ButtonToButtonDistanceY * 2;
	//打ち上げコンボ
	constexpr ComboChainButton kAirCombo[] = { ComboChainButton::X, ComboChainButton::Y,};
	//開始位置
	const int kAirComboStartX = Game::kScreenWidth * 8 / 10;
	const int kAirComboStartY = 100 + ButtonToButtonDistanceY * 3;

	//必殺技3ボタン
	constexpr ComboChainButton kSkillCombo[] = {ComboChainButton::LB, ComboChainButton::X, ComboChainButton::X, ComboChainButton::X };
	//開始位置
	const int kSkillStartX = Game::kScreenWidth * 8 / 10;
	const int kSkillStartY = 100 + ButtonToButtonDistanceY * 4;

	//必殺技ボタン
	constexpr ComboChainButton kUlt[] = { ComboChainButton::LB, ComboChainButton::Y };
	//開始位置
	const int kUltStartX = Game::kScreenWidth * 8 / 10;
	const int kUltStartY = 100 + ButtonToButtonDistanceY * 5;

	//ロックオン
	constexpr ComboChainButton kLockOn = ComboChainButton::RB;
	//開始位置
	const int kLockOnStartX = Game::kScreenWidth * 8 / 10;
	const int kLockOnStartY = 100 + ButtonToButtonDistanceY * 6;
	//ロックオン切換え
	constexpr ComboChainButton kLockOnChange = ComboChainButton::RStickLR;
	//開始位置
	const int kLockOnChangeStartX = Game::kScreenWidth * 8 /10;
	const int kLockOnChangeStartY = 100 + ButtonToButtonDistanceY * 7;

	
}