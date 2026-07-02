#pragma once
class System
{
private:
	//コンストラクタとデストラクタをプライベートにして、シングルトンパターンを実装
	System() = default;
	virtual ~System() = default;
	//コピーコンストラクタと代入演算子を削除して、シングルトンのインスタンスが複製されないようにする
	System(const System&) = delete;
	System& operator=(const System&) = delete;
public:
	//シングルトンインスタンスを取得
	static System& GetInstance()
	{
		static System instance;
		return instance;
	}
	void SetTimeScale(float scale) { timeScale = scale; }
	float GetTimeScale() const { return timeScale; }
	/// <summary>
	/// 時間の流れともとに戻るまでのフレーム数を設定する関数
	/// </summary>
	/// <param name="timescale">時間の流れ=通常時が1.0f</param>
	/// <param name="frames">通常時に戻るまでのframe数</param>
	void SetTimeScaleForFrames(float timescale, int frames)
	{
		timeScale = timescale;//時間のスケールを設定する
		m_frameCount = frames;//フレームカウントを設定する
	}

	void Update();

	void SetUltStart() { m_ultCount = 60; m_isUltimating = true; };
	bool GetIsUltimating() { return m_isUltimating; };
private:
	//時間の管理
	float timeScale = 1.0f;//時間のスケール//1.0fなら通常の時間の流れ//0.5fなら半分の速さ//2.0fなら2倍の速さ

	//いじった時間をもとに戻すためのフレーム
	int m_frameCount = -1;//フレームカウント//ゲームが開始してからのフレーム数//0から始まる

	bool m_isUltimating = false;//必殺技の演出中かどうか
	int m_ultCount = -1;//必殺技の演出時間カウント

};

