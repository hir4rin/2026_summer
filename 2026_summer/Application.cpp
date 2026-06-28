#include "Application.h"
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include <cassert>
#include "Game.h"
#include "Scene/SceneController.h"
#include "Scene/GameScene.h"
#include "DataLoader/DataManager.h"
#include "Input.h"
#include "SubWindow/SubWindow.h"

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
	//SetUseDirect3DVersion(DX_DIRECT3D_11);
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

	//データの読み込み
	DataManager::GetInstance().LoadAll();
	//入力の初期化
	Input::GetInstance().Init();
	//描画先を裏画面に変更する。
	SetDrawScreen(DX_SCREEN_BACK);
	// カリングの設定
	SetUseBackCulling(true);

	// Zバッファを有効にする。
	// Effekseerを使用する場合、2DゲームでもZバッファを使用する。
	SetUseZBuffer3D(TRUE);

	// Zバッファへの書き込みを有効にする。
	// Effekseerを使用する場合、2DゲームでもZバッファを使用する。
	SetWriteZBuffer3D(TRUE);
	return true;
}

void Application::Run()
{
	//シーンコントローラーの作成//最初のシーン生成
	SceneController controller;
#ifdef _DEBUG
	controller.ChangeScene(std::make_shared<GameScene>(controller));
#else
	controller.ChangeScene(std::make_shared<GameScene>(controller));
#endif 

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
		// Effekseerにより再生中のエフェクトを更新する。
		UpdateEffekseer3D();

		Input::GetInstance().Update();
		controller.Update();
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

