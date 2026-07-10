#pragma once
#include "Scene.h"
#include <memory>

class CameraManager;

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
private:
	std::shared_ptr<CameraManager> m_cameraManager;

	int m_titleLogoHandle = -1;

};

