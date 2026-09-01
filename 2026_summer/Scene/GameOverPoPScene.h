#pragma once
#include "Scene.h"
#include <memory>

class GameScene;

//ゲームオーバーを、PauseSceneのようにGameSceneの上に重ねて表示するポップアップシーン
class GameOverPoPScene :
    public Scene
{
public:
    GameOverPoPScene(SceneController& controller, GameScene& gameScene);
    virtual ~GameOverPoPScene();

    void Update() override;
    void FadeInUpdate() override;
    void NormalUpdate() override;
    void FadeOutUpdate() override;

    void Draw() override;
    void FadeInDraw() override;
    void NormalDraw() override;
	void FadeOutDraw() override;

private:
	GameScene& m_gameScene;//積まれた(下にある)GameSceneへの参照

	int m_fadeCount = 0;//フェードイン・フェードアウトの経過フレーム

	int m_deathTextHandle = -1;//画面中央に表示するdeathTextの画像ハンドル
};
