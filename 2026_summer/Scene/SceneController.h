#pragma once
#include <memory>
#include <list>
#include "Player.h"

class Input;
class Scene;
class EffekseerResourceManager;

struct PlayerInformation
{
	PlayerType s_type;
	int s_hp;
	int s_Life;


};

/// <summary>
/// シーンを内部に保持し、切り替えを提供する
/// </summary>
class SceneController
{
public:
	SceneController();
	//プレイヤーの情報を保存
	PlayerInformation m_savePlayer;
	//プレイヤーの情報を更新
	void SetPlayerInfo(PlayerType type,int hp,int life);
	std::shared_ptr<EffekseerResourceManager> m_EfReManager;

private:
	// 現在スタックに積まれているシーン
	// 最後に積んだものだけがUpdateされる
	std::list<std::shared_ptr<Scene>> scenes_;	
public:

	/// <summary>
	/// 強制的なシーンの切り替え(ただし、引数で渡らせたシーンのみになる)
	/// </summary>
	/// <param name="scene">新しいシーン</param>
	/// /// <note>引数で渡されたシーンのみになり、積まれてるシーンの数は1つとなる</note>
	void ResetScene(std::shared_ptr<Scene> scene);

	/// <summary>
	/// シーンの切り替え
	/// </summary>
	/// <param name="scene">新しいシーン</param>
	/// <note>最後に積んだシーンがChangeされる</note>
	void ChangeScene(std::shared_ptr<Scene> scene);

	/// <summary>
	/// シーンを新しく積みます
	/// </summary>
	/// <param name="scene">積みたいシーン</param>
	/// <note>ChangeSceneと違って、もともと積まれてるシーンは残り続ける</note>
	void PushScene(std::shared_ptr<Scene> scene);

	/// <summary>
	/// 最後に積んだシーンをリストから外す(自動的に解放される)
	/// </summary>
	void PopScene();

	/// <summary>
	/// 内部に持ってるシーンのUpdateを呼び出す
	/// </summary>
	void Update(Input& input);

	/// <summary>
	/// 内部に持ってるシーンのDrawを呼び出す
	/// </summary>
	void Draw();
	std::shared_ptr<EffekseerResourceManager> GetEffekseerResourceManager();
};

