#pragma once
#include <windows.h>
#include <string>
/// <summary>
/// SubWindow::Create(hInstance, 1300, 0, 600, 400); //を呼び出すと、指定した位置とサイズでサブウィンドウが作成されるクラス(mainで)
/// </summary>
class SubWindow
{
public:
	static void Create(HINSTANCE hInstance, int x, int y, int width, int height);
	static void SetText(const std::string& text);
	//フレームの最初に呼ぶ
	static void ClearText();
	//文字列を1行ずつ追加していくための関数
	static void AddText(const std::string& text);
	static void Destroy();
	//文字列をサブウィンドウに描画//AddTextで追加された文字列を描画する
	static void Draw();
	
private:
	static HWND m_hWnd;
	static HWND m_hLabel;
	static std::string m_buffer;//クラス内で初期化はできない
};

