#pragma once
#include "PlayerAnimData.h"
#include <vector>
#include <string>

/// <summary>
/// ゲーム全体のCSVデータの管理を行うクラス
/// 起動時にLoadAll()を呼び出して、全てのCSVデータを読み込むことを想定している
/// </summary>
class DataManager
{
public:
	static DataManager& GetInstance()
	{
		static DataManager instance;
		return instance;
	}
	/// <summary>
	/// 起動時に一度だけ呼び出す
	/// </summary>
	void LoadAll();

	const PlayerAnimData& GetPlayerAnimData() const { return m_playerAnimData;}
	const std::vector<std::vector<std::string>>& GetComboRawData() const { return m_comboRawData;}	
	//今後追加する場合はここにgetterを追加する
private:
	//シングルトンパターンの実装
	DataManager() = default;
	~DataManager() = default;
	DataManager(const DataManager&) = delete;
	DataManager& operator=(const DataManager&) = delete;

	void LoadPlayerAnimData(); //プレイヤーのアニメーションデータを読み込む関数
	void LoadComboRawData(); //コンボの生データを読み込む関数
	//今後追加する場合はここにLoad関数を追加する

	PlayerAnimData m_playerAnimData;//プレイヤーのアニメーションデータ
	std::vector<std::vector<std::string>> m_comboRawData;//コンボの生データ//コンボチェーンのデータを読み込む予定
	//今後追加する場合はここにデータのメンバ変数を追加する

};

