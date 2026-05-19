#pragma once
#include "Player.h"//playerの状態を継承先が渡す用
class SceneController; // シーンコントローラーのプロトタイプ宣言
class Input; // 入力のプロトタイプ宣言


constexpr int cameraMargin = 100;
constexpr int screenWidth = 1920;
constexpr int screenHeight = 1080;



/// <summary>
/// シーン基底クラス
/// </summary>
class Scene
{
protected:
	SceneController& controller_;
public:
	Scene(SceneController& controller);
	/// <summary>
	/// シーンの情報の更新
	/// </summary>
	/// <param name="input">入力情報</param>
	virtual void Update(Input& input) = 0;

	/// <summary>
	/// シーンの描画(シーンの持ち物も描画する)
	/// </summary>
	virtual void Draw() = 0;
};

