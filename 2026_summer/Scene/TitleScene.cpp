#include "TitleScene.h"
#include "../Game.h"
#include "../System.h"
#include "../Input.h"
#include"SceneController.h"
#include "GameScene.h"
#include "../Camera/CameraManager.h"
#include "../Camera/TitleCamera.h"
#include "../Managers/CollisionManager.h"
#include "Player.h"


namespace
{
	constexpr int kTitleLogoX = 707;
	constexpr int kTitleLogoY = 275;
}


TitleScene::TitleScene(SceneController& controller) :Scene(controller)
{
	m_updateFunc = static_cast<UpdateFunc_t>(&TitleScene::NormalUpdate);
	m_drawFunc = static_cast<DrawFunc_t>(&TitleScene::NormalDraw);

	//dataの読み込み
	m_titleLogoHandle = LoadGraph("data/UI/TitleLogo.png");

	//プレイヤーの初期化
	m_player = std::make_shared<Player>();
	m_player->Init();
	m_player->SetPos(Vector3(0,0,300));
	m_player->SetIsTitleMode(true);//タイトル画面では移動方向をワールド座標の固定軸にする
	InputInitialize();

	m_cameraManager = std::make_shared<CameraManager>();
	m_cameraManager->SetIsTitle(true);//タイトル画面ではTitleCameraのみを使う
	//カメラの初期化
	m_cameraManager->Init(std::weak_ptr<Player>(m_player));
	m_cameraManager->Update(m_player->GetPos());
	//プレイヤーにカメラマネージャーの弱参照を渡す
	m_player->SetCameraManager(std::weak_ptr<CameraManager>(m_cameraManager));
	//タイムスケールを掛ける
	//System::GetInstance().SetTimeScale(0.5f);
}

TitleScene::~TitleScene()
{
	//dataの解放
	DeleteGraph(m_titleLogoHandle);
}

void TitleScene::Update()
{
	(this->*m_updateFunc)();
}

void TitleScene::FadeInUpdate()
{
}

void TitleScene::NormalUpdate()
{

	auto& input = Input::GetInstance();

	m_cameraManager->Update(m_player->GetPos());

	m_player->Update(*m_cameraManager->GetHighestPriorityCamera());
	CollisionManager::GetInstance().Update();
	if(input.IsRealTriggered("A"))
	{
		Input::GetInstance().StopRecord();

		//ゲームシーンに遷移する
		m_controller.ChangeScene(std::make_shared<GameScene>(m_controller));
		return;
	}
	if (input.IsRealTriggered("B"))
	{
		std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
		TitleCamera::Shot shot = TitleCamera::Shot::FollowPlayer;
		titleCamera->SetShot(shot);
		return;
	}
	if (input.IsRealTriggered("Y"))
	{
		std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
		TitleCamera::Shot shot = TitleCamera::Shot::Fixed;
		titleCamera->SetShot(shot);
		return;
	}
}

void TitleScene::FadeOutUpdate()
{
}

void TitleScene::Draw()
{
	(this->*m_drawFunc)();
}

void TitleScene::FadeInDraw()
{
}

void TitleScene::NormalDraw()
{
	SetDrawScreen(DX_SCREEN_BACK); ClearDrawScreen();
	m_cameraManager->ApplyCameraSettings();
	DrawRectRotaGraph(Game::kScreenWidth /2, Game::kScreenHeight / 2,0,0,kTitleLogoX,kTitleLogoY,
		1.0, 0.0, m_titleLogoHandle, TRUE);

	m_player->Draw();
#ifdef _DEBUG
	m_cameraManager->Draw();
#endif
}

void TitleScene::FadeOutDraw()
{
}

void TitleScene::InputInitialize()
{
	const int button = 20;
	const int space = 10;
	InputRecord record;
	record = {
		{200,{"Right"}},
		{400,{}},
		{2,{"Right"}},
		{button,{"X"}},
		{space,{}},
		{button,{"X"}},
		{space,{}},
		{button,{"X"}},
		{space,{}},
		{button,{"X"}},
		{space,{}},
		{button,{"X"}},
		{space,{}},
		{button,{"X"}},
		{space,{}},
		{button,{"X"}},
	};
	Input::GetInstance().StartRecord(record);
}
