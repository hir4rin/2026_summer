#include "SubWindow.h"

//staticメンバ変数の定義
HWND SubWindow::m_hWnd = nullptr;//Window本体
HWND SubWindow::m_hLabel = nullptr;//テキストを表示するためのラベル
std::string SubWindow::m_buffer = "";//AddText用の文字列バッファ


void SubWindow::Create(HINSTANCE hInstance, int x, int y, int width, int height)
{
	// 独自クラスを登録（DefWindowProcAが移動などを処理してくれる）
	WNDCLASSEXA wc = {};
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.lpfnWndProc = DefWindowProcA; // 移動・リサイズなどを自動処理
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = "SubWindowClass";
	RegisterClassExA(&wc);

	m_hWnd = CreateWindowExA(
		0, "SubWindowClass",  // ← 独自クラスを使用
		"SubWindow",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		x, y, width, height,
		NULL, NULL, hInstance, NULL
	);

	m_hLabel = CreateWindowExA(
		0, "STATIC", "",
		WS_CHILD | WS_VISIBLE | SS_LEFT,
		10, 20, width - 20, height - 20,
		m_hWnd, NULL, hInstance, NULL
	);


	// 独自クラスを登録しない場合（DefWindowProcAが移動などを処理してくれないため、ウィンドウの移動やリサイズができなくなる）
	//原点

	//m_hWnd = CreateWindowEx(
	//	0, "STATIC",
	//	"SubWIndow",
	//	WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	//	x, y, width, height,
	//	NULL, NULL, hInstance, NULL
	//);

	//// ラベルを追加
	//m_hLabel = CreateWindowEx(
	//	0, "STATIC", "",
	//	WS_CHILD | WS_VISIBLE | SS_LEFT,
	//	10, 20, width - 20, height - 20,
	//	m_hWnd,//親ウィンドウを指定
	//	NULL, hInstance, NULL
	//);
	
}

void SubWindow::SetText(const std::string& text)
{
	if (m_hLabel)
	{
		//原点
		//SetWindowText(m_hLabel, text.c_str());

		 // タイトルバーではなく、クライアント領域に直接描画
		HDC hdc = GetDC(m_hWnd);
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		SetBkColor(hdc, RGB(0, 0, 0));    // 背景：黒
		SetTextColor(hdc, RGB(0, 255, 0)); // 文字：緑
		FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
		DrawTextA(hdc, text.c_str(), -1, &rect, DT_LEFT | DT_TOP);
		ReleaseDC(m_hWnd, hdc);
	}
}

void SubWindow::ClearText()
{
	m_buffer = ""; // フレームの最初にクリア
}

void SubWindow::AddText(const std::string& text)
{
	m_buffer += text + "\n"; // 文字列をバッファに追加

}

void SubWindow::Destroy()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}
}

void SubWindow::Draw()
{
	//バッファの内容をウィンドウに描画
	//バッファ全体を毎回描画し直す
	HDC hdc = GetDC(m_hWnd);
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(0, 255, 0));
	FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	DrawTextA(hdc, m_buffer.c_str(), -1, &rect, DT_LEFT | DT_TOP);
	ReleaseDC(m_hWnd, hdc);
}
