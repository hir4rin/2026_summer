#pragma once
#include <string>
/// <summary>
/// Animationクラス(アニメーションさせるものに持たせる)
/// </summary>
class Animation
{
public:
	Animation();
	virtual ~Animation();
	/// <summary>
	/// handleとnameを受け取ってアニメーションの初期化を行う
	/// </summary>
	/// <param name="modelHandle"></param>
	/// <param name="name"></param>
	void Init(int modelHandle,std::string name,bool isRoop,float timescale = 1.0f);
	
	void Update();//ここにtimeScaleを引数にして渡して、することで、プレイヤーでもエネミーでも使うと一緒に使える
	void AnimBlend();//アニメーションのブレンドを行う

	void SetAnim(bool isRoop);//アニメーションのループ再生を設定する//m_isEndを初期化
	void ChangeAnim(std::string name,bool isRoop = true,float timescale = 1.0f);//アニメーションを切り替える//nameはアニメーションの名前

	bool GetAnimEndFlag() { return m_isEndAnim;}//アニメーションが終わったかどうかのフラグを返す
	float GetAnimRate();//アニメーションの進行率を返す//0から1の値を返す

private:
	int m_modelHandle;//モデルのハンドル
	int m_currentAnimHandle;//現在のアニメーションのハンドル
	int m_prevAnimHandle;//前のアニメーションのハンドル

	float m_currentAnimCount;//現在のアニメーションのフレーム数
	float m_prevAnimCount;//前のアニメーションのフレーム数

	float m_animChangeFrame;//アニメーションを切り替えるフレーム数

	bool m_isRoop;//アニメーションをループさせるかどうかのフラグ
	bool m_prevRoop;//前のアニメーションがループしているかどうかのフラグ
	bool m_isEndAnim;//アニメーションが終わったかどうかのフラグ

	float m_animtimeScale = 1.0f;//アニメーションの再生速度を管理するための変数//1.0fが通常の速度で、0.5fなら半分の速度、2.0fなら倍の速度になる
	float m_prevAnimTimeScale = 1.0f;//前のアニメーションの再生速度を管理するための変数


};

