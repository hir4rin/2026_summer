#pragma once
#include "Scene.h"
class GameScene : public Scene
{
public :
		GameScene(SceneController& controller);
		void Update(Input& input) override;
		void Draw() override;
private:

};

