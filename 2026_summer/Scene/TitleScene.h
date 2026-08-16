#pragma once
#include "Scene.h"
#include <memory>

class CameraManager;
class Player;
class Weapon;
class Stage;
class SpeedLine2D;
class SkyBox;
class Titlemascot;

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
	void CameraSetUpdate();
private:
	std::shared_ptr<CameraManager> m_cameraManager;
	std::shared_ptr<Player> m_player;
	std::shared_ptr< Weapon> m_weapon;
	std::shared_ptr<Stage> m_stage;
	std::shared_ptr<SpeedLine2D> m_speedLine;
	std::shared_ptr<SkyBox> m_skyBox;
	std::shared_ptr<Titlemascot> m_mascot;

	int m_titleLogoHandle = -1;
	int m_count = 0;

};

