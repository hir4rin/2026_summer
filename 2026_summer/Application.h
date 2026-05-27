#pragma once
#include <windows.h>


struct Size
{
	int w;
	int h;
};

class Application
{

private:
	Size m_windowSize;

	Application();
	Application(const Application& app) = delete;//コピーコンストラクタも//privateにして複製を防ぐ
	//コピーを防ぐ
	void operator=(const Application& app) = delete; //=deleteをつけると
	                                               //何をしてもこれを使えなくなる
	bool m_requestedExit = false;//ゲームの終了をがリクエスト

public:
	~Application();
	static Application& GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <returns>初期化成功:true / 初期化失敗:false</returns>
	bool Init();

	/// <summary>
	/// アプリケーションをスタートする
	/// ゲームループを終了する
	/// </summary>
	void Run();
	/// <summary>
	/// アプリケーションの終了
	/// </summary>
	void Terminate();
	/// <summary>
	/// ウィンドウサイズを返す
	/// </summary>
	/// <returns></returns>
	const Size& GetWindowSize() const;
	/// <summary>
	/// アプリケーションを終了する命令を飛ばす
	/// </summary>
	void RequestExit();
	/// <summary>
	/// サブウィンドウの作成
	/// </summary>
	/// <param name="hInstance">WinMainから渡されるインスタンスハンドル</param>
	void CreateSubWindow(HINSTANCE hInstance);
};





