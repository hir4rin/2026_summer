#include "GameScene.h"
#include "../Camera/CameraManager.h"
#include "Player.h"
#include "../Character/Enemy/Swordman/EnemySwordman.h"

namespace
{
	constexpr int kGridRange = 2400;//グリッドのサイズ
}

GameScene::GameScene(SceneController& controller):Scene(controller)
{
	//基底クラスに継承先のポインタをキャストして代入する
	m_updateFunc = static_cast<UpdateFunc_t>(&GameScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&GameScene::NormalDraw);

	//プレイヤーの初期化
	m_player = std::make_shared<Player>();
	m_player->Init();
	//敵の初期化
	m_enemySwordman = std::make_shared<EnemySwordman>(std::weak_ptr<Player>(m_player));
	m_enemySwordman->Init();

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
	m_cameraManager->Update(m_player->GetPos() - Vector3(0,m_player->GetCenter(),0));
	m_player->Update(*m_cameraManager->GetHighestPriorityCamera());
	m_enemySwordman->Update();
	CollisionManager::GetInstance().Update();
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
	DrawFormatString(300, 0, GetColor(255, 255, 255), "GameScene");
	DrawGrid();

	m_player->Draw();
	m_enemySwordman->Draw();

#ifdef _DEBUG
	CollisionManager::GetInstance().DebugDraw();
#endif
}

void GameScene::FadeOutDraw()
{
}

void GameScene::DrawGrid()
{
	// 直線の始点と終点
	VECTOR startPos;
	VECTOR endPos;

	for (int z = -kGridRange; z <= kGridRange; z += 100)
	{
		startPos = VGet(-kGridRange, 0.0f, static_cast<float>(z));
		endPos = VGet(kGridRange, 0.0f, static_cast<float>(z));
		DrawLine3D(startPos, endPos, 0xff0000);
	}
	for (int x = -kGridRange; x <= kGridRange; x += 100)
	{
		startPos = VGet(static_cast<float>(x), 0.0f, -kGridRange);
		endPos = VGet(static_cast<float>(x), 0.0f, kGridRange);
		DrawLine3D(startPos, endPos, 0x0000ff);
	}
}
