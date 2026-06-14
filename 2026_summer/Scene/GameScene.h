#pragma once
#include <memory>
#include "Scene.h"


class CameraManager;
class Player;
class EnemySwordman;

class GameScene : public Scene
{
public :
		GameScene(SceneController& controller);
		~GameScene();
		void Update() override;
		void FadeInUpdate() override;
		void NormalUpdate() override;
		void FadeOutUpdate() override;

		void Draw() override;
		void FadeInDraw() override;
		void NormalDraw() override;
		void FadeOutDraw() override;
private:
	void DrawGrid();
private:
	std::unique_ptr<CameraManager> m_cameraManager;
	std::shared_ptr<Player> m_player;
	std::shared_ptr<EnemySwordman> m_enemySwordman;
};

