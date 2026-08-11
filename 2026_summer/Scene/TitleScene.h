#pragma once
#include "Scene.h"
#include <memory>

class CameraManager;
class Player;

class TitleScene :
    public Scene
{
public:
	TitleScene(SceneController& controller);
	virtual ~TitleScene();

	void Update() override;
	void FadeInUpdate() override;
	void NormalUpdate() override;
	void FadeOutUpdate() override;

	void Draw() override;
	void FadeInDraw() override;
	void NormalDraw() override;
	void FadeOutDraw() override;

	void InputInitialize();
private:
	std::shared_ptr<CameraManager> m_cameraManager;
	std::shared_ptr<Player> m_player;

	int m_titleLogoHandle = -1;

};

