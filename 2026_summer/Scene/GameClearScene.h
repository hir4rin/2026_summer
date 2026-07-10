#pragma once
#include "Scene.h"
#include <memory>

class CameraManager;

class GameClearScene :
    public Scene
{
public:
    GameClearScene(SceneController& controller);
    virtual ~GameClearScene();

    void Update() override;
    void FadeInUpdate() override;
    void NormalUpdate() override;
    void FadeOutUpdate() override;

    void Draw() override;
    void FadeInDraw() override;
    void NormalDraw() override;
	void FadeOutDraw() override;
private:
	std::shared_ptr<CameraManager> m_cameraManager;

};

