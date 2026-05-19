#include "SceneController.h"
#include "EffekseerResourceManager.h"
#include "Scene.h"

SceneController::SceneController()
{
	m_EfReManager = std::make_shared< EffekseerResourceManager>();
}

void SceneController::SetPlayerInfo(PlayerType type, int hp, int life)
{
	m_savePlayer.s_type = type;
	m_savePlayer.s_hp = hp;
	m_savePlayer.s_Life = life;
}



void SceneController::ResetScene(std::shared_ptr<Scene> scene)
{
	// シーンを一つだけにしたいので、いったんシーンをすべてクリアします
	scenes_.clear();
	scenes_.push_back(scene);
}

void SceneController::ChangeScene(std::shared_ptr<Scene> scene)
{
	// もし、リストが空の場合、普通に代入でChangeSceneしようとすると、当然emptyの箱に対してChangeしようとするので、クラッシュします。
	// このため、最初にemptyかどうかをチェックします
	if (scenes_.empty())	// 最初は要素がないためpush_backで要素数を増やす
	{
		// 空っぽの場合には指定の要素をpushします。少なくとも一つは積まれている状態にする
		scenes_.push_back(scene);
	}
	else
	{
		scenes_.back() = scene;	// この行の時点で元のシーンは自動的に削除されています
	}
}

void SceneController::PushScene(std::shared_ptr<Scene> scene)
{
	// 新しいシーンを末尾に積みます
	scenes_.push_back(scene);
}

void SceneController::PopScene()
{
	// 末尾のシーンを削除します
	scenes_.pop_back();
}

void SceneController::Update(Input& input)
{
	// 末尾の要素に対してのみUpdateする
	scenes_.back()->Update(input);
}

void SceneController::Draw()
{
	// 末尾の要素に対してのみDrawする
	for (auto& scene : scenes_)
	{
		scene->Draw();
	}
}

std::shared_ptr<EffekseerResourceManager> SceneController::GetEffekseerResourceManager()
{
	return m_EfReManager;
}
