#include "Application.h"
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include <cassert>
#include <unordered_map>
#include <string>
#include "Game.h"
#include "Scene/SceneController.h"
#include "Scene/GameScene.h"
#include "Scene/TitleScene.h"
#include "DataLoader/DataManager.h"
#include "Input.h"
#include "SubWindow/SubWindow.h"
#include "System.h"

constexpr int kWindowSizeW = 1920;	// デフォルトウィンドウ幅
constexpr int kWindowSizeH = 1080;	// デフォルトウィンドウ高
constexpr int kColorBit = 32;	// デフォルトカラービット

Application::Application() :
	m_windowSize{ kWindowSizeW,kWindowSizeH }
{

}
Application::~Application()
{
}

Application& Application::GetInstance()
{
	// staticなのでメモリの場所が一つで確定→これ一つしか実態がない→シングルトンクラス
	static Application instance;
	return instance;	// インスタンスの参照を返す→Applicationクラスの場所を返す
	// staticで一つしか存在しないものの参照を返すので複数になることはない
}

bool Application::Init()
{
	//ウィンドウモード設定
	ChangeWindowMode(true);
	//タイトル変更
	SetMainWindowText("NINJA SCARLET");
	//画面のサイズ変更
	SetGraphMode(Game::kScreenWidth, Game::kScreenHeight, Game::kColorBitNum);
	

	// DirectX11を使用するようにする。(DirectX9も可、一部機能不可)
	// Effekseerを使用するには必ず設定する。
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}
	// Effekseerを初期化する。
	// 引数には画面に表示する最大パーティクル数を設定する。
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
		return -1;
	}
	// フルスクリーンウインドウの切り替えでリソースが消えるのを防ぐ。
	// Effekseerを使用する場合は必ず設定する。
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	// DX11使用時、Effekseerのガンマカラーをリニア空間で維持するよう設定する
	// これがないとテクスチャが白飛びして見える
	//GetEffekseer3DRenderer()->SetMaintainGammaColorInLinearColorSpace(true);

	//データの読み込み
	DataManager::GetInstance().LoadAll();
	//入力の初期化
	Input::GetInstance().Init();
	//描画先を裏画面に変更する。
	SetDrawScreen(DX_SCREEN_BACK);
	// カリングの設定
	SetUseBackCulling(true);

	// Zバッファへの書き込みを有効にする。
	// Effekseerを使用する場合、2DゲームでもZバッファを使用する。
	SetWriteZBuffer3D(TRUE);

	// Zバッファを有効にする。
	// Effekseerを使用する場合、2DゲームでもZバッファを使用する。
	SetUseZBuffer3D(TRUE);
	return true;
}

void Application::Run()
{
	//シーンコントローラーの作成//最初のシーン生成
	SceneController controller;
#ifdef _DEBUG
	controller.ChangeScene(std::make_shared<TitleScene>(controller));
#else
	controller.ChangeScene(std::make_shared<TitleScene>(controller));
#endif 
	//player,enemy,stage,weapon,UI画像

	//非同期ロード
	AsyncLoad();

	while (ProcessMessage() == 0 && !m_requestedExit)
	{
		



		LONGLONG time = GetNowHiPerformanceCount();
		//画面のクリア
		ClearDrawScreen();
		//サブウィンドウのテキストをクリア
		SubWindow::ClearText();

		if (CheckHitKey(KEY_INPUT_R))
		{
			//Rキーでシーンリセット
			/*if (pSceneMain != nullptr)
			{
				delete pSceneMain;
				pSceneMain = nullptr;
				pSceneMain = new SceneMain;
				pSceneMain->Init();
			}*/
		}

		Input::GetInstance().Update();
		controller.Update();
		// Effekseerにより再生中のエフェクトを更新する。
		UpdateEffekseer3D();
		controller.Draw();
		SubWindow::Draw();//サブウィンドウの描画
		//裏画面と表画面を入れ替える
		ScreenFlip();

		if (CheckHitKey(KEY_INPUT_ESCAPE))//escapeで抜け出す
		{
			break;
		}

		//fpsを60に固定
		while (GetNowHiPerformanceCount() - time < 16667)
		{

		}
	}
}

void Application::Terminate()
{
	//非同期ロードのハンドルを削除する
	System::GetInstance().SetTerminate();

	// Effekseerを終了する。
	Effkseer_End();

	DxLib_End();
}

const Size& Application::GetWindowSize() const
{
	
	return m_windowSize;
}

void Application::RequestExit()
{
	m_requestedExit = true;
}

void Application::CreateSubWindow(HINSTANCE hInstance)
{
	//サブウィンドウの作成//引数で位置とサイズを指定できるようにする
	SubWindow::Create(hInstance, 1300, 0, 600, 400);
}

void Application::AsyncLoad()
{
	// これがロード処理より前に呼ばれているか確認
	SetUseASyncLoadFlag(TRUE);
	//ロード
	std::unordered_map<AsyncData, int> handleData;
	//player
	handleData[AsyncData::PlayerModel] = MV1LoadModel("data/Player/Player_Init.mv1");
	handleData[AsyncData::PlayerAttackModel] = MV1LoadModel("data/Player/Sonoda/sonoda.mv1");
	handleData[AsyncData::PlayerWeaponModel] = MV1LoadModel("data/Player/Weapon/red_katana.mv1");
	handleData[AsyncData::PlayerWingModel] = MV1LoadModel("data/Player/Weapon/Wing/wing.mv1");
	handleData[AsyncData::PlayerEffectSkill] = LoadEffekseerEffect("data/Effect/Slash.efk", 1.0f);
	//enemy
	handleData[AsyncData::EnemyModel] = MV1LoadModel("data/Enemy/sasakiPlayer.mv1");
	//stage
	handleData[AsyncData::StageModel] = MV1LoadModel("data/Stage_Graphic/Stage.mv1");
	handleData[AsyncData::StageModelCollider] = MV1LoadModel("data/Stage_Graphic/coll.mv1");

	int totalNum = handleData.size();

	while (ProcessMessage() == 0 && !m_requestedExit)
	{
		//非同期ロード
		int loadingNum = GetASyncLoadNum();//非同期ロード中の数を取得
		if (loadingNum == 0) 
		{
			break;
		}
		ClearDrawScreen();
		//現在のロード状況を表示する
		DrawFormatString(Game::kScreenWidth / 2, Game::kScreenHeight / 2, GetColor(255, 255, 255), "Loading... %d / %d", totalNum - loadingNum, totalNum);

		ScreenFlip();


	}
	SetUseASyncLoadFlag(false);
	//システムにデータを渡す
	System::GetInstance().SetHandleData(handleData);

}



