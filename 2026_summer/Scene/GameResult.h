#pragma once

//リザルト画面に表示するプレイ結果
struct GameResult
{
	float clearTime = 0.0f;//クリアタイム(秒)
	int totalDamage = 0;//与えた合計ダメージ
	int comboCount = 0;//コンボ数(総ヒット数)
	int damageTakenCount = 0;//被弾回数
};
