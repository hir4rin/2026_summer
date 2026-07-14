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

enum class ComboButton : int
{
	X = 0,
	Y = 1,
};

namespace ComboUI
{
	constexpr int kButtonScale = 1.0f;//ボタンの大きさ//倍率

	//弱5ボタン
	constexpr ComboButton kCombo1[] = { ComboButton::X, ComboButton::X, ComboButton::X,ComboButton::X,ComboButton::X };
	//開始位置
	constexpr int kCombo1StartX = 1110;
	constexpr int kCombo1StartY = 10;


	//強2ボタン
	constexpr ComboButton kCombo2[] = { ComboButton::Y, ComboButton::Y };
	//開始位置
	constexpr int kCombo2StartX = 1110;
	constexpr int kCombo2StartY = 100;

	//必殺技3ボタン
	constexpr ComboButton kSkill[] = { ComboButton::X, ComboButton::Y, ComboButton::X };
	//開始位置
	constexpr int kSkillStartX = 1110;
	constexpr int kSkillStartY = 200;

	//必殺技ボタン
	constexpr ComboButton kUlt[] = { ComboButton::Y };
	//開始位置
	constexpr int kUltStartX = 1110;
	constexpr int kUltStartY = 300;
}