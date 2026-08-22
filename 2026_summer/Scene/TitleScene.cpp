#include "TitleScene.h"
#include "../Game.h"
#include "../System.h"
#include "../Input.h"
#include"SceneController.h"
#include "GameScene.h"
#include "../Camera/CameraManager.h"
#include "../Camera/TitleCamera.h"
#include "../Camera/Camera.h"
#include "../Camera/MainCamera.h"
#include "../Managers/CollisionManager.h"
#include "../Stage/SkyBox.h"
#include "Player.h"
#include "../Weapon.h"
#include "../SpeedLine2D.h"
#include "../Stage/Stage.h"
#include "../Character/Mascot/Titlemascot.h"

namespace
{

	constexpr int kTitleLogoX = 707;
	constexpr int kTitleLogoY = 275;

	constexpr int kCameraSetUp = 1000;

	//剣の座標(Weapon::TitleUpdateで刺さっている位置と合わせる)から見て右側(TitleCameraのFixedショットで画面右)の座標
	const Vector3 kMascotPos = Vector3(-10.0f, 0.0f, -5050.0f);
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

	m_weapon = std::make_shared<Weapon>(std::weak_ptr(m_player));

	m_stage = std::make_shared<Stage>();
	m_stage->TitleInit();

	m_speedLine = std::make_shared<SpeedLine2D>();

	//剣を見ているネズミ//剣の右らへんに配置する
	m_mascot = std::make_shared<Titlemascot>(kMascotPos);
	m_mascot->Init();

	m_cameraManager = std::make_shared<CameraManager>();
	m_cameraManager->SetIsTitle(true);//タイトル画面ではTitleCameraのみを使う
	//カメラの初期化
	m_cameraManager->Init(std::weak_ptr<Player>(m_player));
	m_cameraManager->Update(m_player->GetPos());
	//タイトルカメラの初期設定
	std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
	TitleCamera::Shot shot = TitleCamera::Shot::FollowPlayer;
	titleCamera->SetShot(shot);
	//プレイヤーにカメラマネージャーの弱参照を渡す
	m_player->SetCameraManager(std::weak_ptr<CameraManager>(m_cameraManager));

	m_skyBox = std::make_shared<SkyBox>();
	m_skyBox->Init(std::weak_ptr<Camera>(m_cameraManager->GetMainCamera()));
	//タイムスケールを掛ける
	//System::GetInstance().SetTimeScale(1.15f);
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

	m_count++;
	CameraSetUpdate();

	m_speedLine->TryGenerate(true);
	m_speedLine->Update();
	

	m_player->Update(*m_cameraManager->GetHighestPriorityCamera());

	if (m_player->GetPos().z <= -5050.0f)
	{
		Titlemascot::State state = Titlemascot::State::Kirimomi;
		m_mascot->SetState(state);
	}


	m_weapon->TitleUpdate();
	m_mascot->Update();

	CollisionManager::GetInstance().Update();

	m_skyBox->Update();

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
	m_skyBox->Draw();
	m_stage->Draw();
	if (m_count >= kCameraSetUp)
	{
		DrawRectRotaGraph(Game::kScreenWidth / 2, Game::kScreenHeight / 4, 0, 0, kTitleLogoX, kTitleLogoY,
		1.0, 0.0, m_titleLogoHandle, TRUE);
	}
	
	{
		std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
		TitleCamera::Shot shot = TitleCamera::Shot::FollowPlayer;
		if (titleCamera->GetShot() != TitleCamera::Shot::Fixed && titleCamera->GetShot() != TitleCamera::Shot::Finish)
		{
			m_speedLine->Draw();
		}
	}

	m_player->Draw();
	m_weapon->TitleDraw();
	m_mascot->Draw();
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
	//外部読み込みを後でしとく
	record = {
		{kCameraSetUp,{"Down"}},
		{8000,{}},
		{2,{"Down"}},
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

void TitleScene::CameraSetUpdate()
{
	//カウントの進み具合によってカメラのステートを更新する
	if (m_count == 100)
	{
		std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
		TitleCamera::Shot shot = TitleCamera::Shot::Fixed;
		titleCamera->SetShot(shot);
	}
	if (m_count == 300)
	{
		std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
		TitleCamera::Shot shot = TitleCamera::Shot::FollowPlayer;
		titleCamera->SetShot(shot);
	}
	if (m_count == 400)
	{
		std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
		TitleCamera::Shot shot = TitleCamera::Shot::Fixed;
		titleCamera->SetShot(shot);
	}
	if (m_count == 600)
	{
		std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
		TitleCamera::Shot shot = TitleCamera::Shot::ZoomOut;
		titleCamera->SetShot(shot);
	}

	if (m_count == kCameraSetUp)
	{
		std::shared_ptr<TitleCamera>  titleCamera = std::dynamic_pointer_cast<TitleCamera>(m_cameraManager->GetHighestPriorityCamera());
		TitleCamera::Shot shot = TitleCamera::Shot::Opening;
		titleCamera->SetShot(shot);
	}

}
