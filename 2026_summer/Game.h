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
	constexpr float kMoveSpeed = 6.0f;//移動速度
	constexpr float kAirMaxSpeed = 5.5f;//空中での最大移動速度
}