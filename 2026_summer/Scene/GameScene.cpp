#include "GameScene.h"
#include "../Camera/CameraManager.h"
#include "../Character/Player/Player.h"

GameScene::GameScene(SceneController& controller):Scene(controller)
{
	//基底クラスに継承先のポインタをキャストして代入する
	m_updateFunc = static_cast<UpdateFunc_t>(&GameScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameScene::NormalDraw);

	//プレイヤーの初期化
	m_player = std::make_shared<Player>();
	m_player->Init();

	m_cameraManager = std::make_unique<CameraManager>();
	//カメラの初期化
	m_cameraManager->Update(m_player->GetPos());
}
GameScene::~GameScene()
{

}

void GameScene::Update()
{
	(this->*m_updateFunc)();
}

void GameScene::FadeInUpdate()
{
}

void GameScene::NormalUpdate()
{
	m_cameraManager->Update(m_player->GetPos());
	m_player->Update(*m_cameraManager->GetHighestPriorityCamera());
}

void GameScene::FadeOutUpdate()
{
}

void GameScene::Draw()
{
	(this->*m_drawFunc)();
}

void GameScene::FadeInDraw()
{
}

void GameScene::NormalDraw()
{
	DrawFormatString(0, 0, GetColor(255, 255, 255), "GameScene");
	DrawGrid();

	m_player->Draw();
}

void GameScene::FadeOutDraw()
{
}

void GameScene::DrawGrid()
{
	// 直線の始点と終点
	VECTOR startPos;
	VECTOR endPos;

	for (int z = -300; z <= 300; z += 100)
	{
		startPos = VGet(-300.0f, 0.0f, static_cast<float>(z));
		endPos = VGet(300.0f, 0.0f, static_cast<float>(z));
		DrawLine3D(startPos, endPos, 0xff0000);
	}
	for (int x = -300; x <= 300; x += 100)
	{
		startPos = VGet(static_cast<float>(x), 0.0f, -300.0f);
		endPos = VGet(static_cast<float>(x), 0.0f, 300.0f);
		DrawLine3D(startPos, endPos, 0x0000ff);
	}
}
