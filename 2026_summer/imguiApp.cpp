#include "imguiApp.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include <d3d11.h>
#include <cmath>
#include <algorithm>

//imgui_impl_win32.hはwindows.hへの依存を避けるため、
//WndProcHandlerの宣言をコメントアウトしている。使う側でこの宣言をコピーする必要がある。
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
	constexpr float kPi = 3.14159265358979323846f;
	constexpr float kDegToRad = DX_PI_F / 180.0f;//度→ラジアン変換用(sinf/cosfはラジアン単位でしか計算できないため)
	constexpr float kRadToDeg = 180.0f / DX_PI_F;//ラジアン→度変換用(画面表示は度のほうが人間に分かりやすいため)

	const ImVec2 kCameraDebugItemSpacing = ImVec2(8.0f, 16.0f);//カメラデバッグウィンドウ内の項目間隔(横,縦)

	constexpr float kPosDragSpeed = 1.0f;//座標(Position/Target)のドラッグ感度
	constexpr float kAngleDragSpeed = 0.5f;//角度(Angle H/V)のドラッグ感度
	constexpr float kAngleHDegMin = -180.0f;//水平角度の最小値(度)
	constexpr float kAngleHDegMax = 180.0f;//水平角度の最大値(度)
	constexpr float kAngleVDegMin = -89.0f;//垂直角度の最小値(度)
	constexpr float kAngleVDegMax = 89.0f;//垂直角度の最大値(度)
	constexpr float kDistanceDragSpeed = 1.0f;//距離のドラッグ感度
	constexpr float kDistanceMin = 10.0f;//注視点までの距離の最小値
	constexpr float kDistanceMax = 5000.0f;//注視点までの距離の最大値

	constexpr float kMinDistanceEpsilon = 0.0001f;//距離がこれ以下だと0割りに近くなり不安定になるため計算を中断する閾値
}

//ImGuiを使えるようにする初期化処理。ここで「ImGuiの入れ物を作る」→「入力と描画の係(バックエンド)を登録する」→
//「DxLibのウィンドウにImGuiの入力処理を割り込ませる」の3段階を行う
void imguiApp::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
	if (m_initialized)return;//すでに初期化済みなら何もしない(二重初期化防止)

	IMGUI_CHECKVERSION();//ヘッダファイルとライブラリ本体のバージョンが一致しているかチェックする(imgui定番のおまじない)
	ImGui::CreateContext();//ImGuiが内部で使うデータ(フォント、各ウィンドウの状態など)の入れ物を新しく作る
	ImGuiIO& io = ImGui::GetIO();//入出力まわりの設定を取得する
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;//キーボードだけでもImGuiのUIを操作できるようにする設定

	ImGui::StyleColorsDark();//見た目のテーマをダーク(黒っぽい配色)にする

	ImGui_ImplWin32_Init(hwnd);//「このウィンドウのマウス/キーボード入力を見るよ」とImGuiに教える(入力係の登録)
	ImGui_ImplDX11_Init(device, context);//「この描画デバイスを使って絵を出すよ」とImGuiに教える(描画係の登録)

	//DxLibが内部で作ったウィンドウプロシージャに、ImGuiの入力処理を割り込ませる
	//SetWindowLongPtrで「次からこのウィンドウ宛てのメッセージはWndProcHandlerに先に渡してね」と入れ替えている
	//戻り値(入れ替え前の関数)をm_prevWndProcに保存しておき、WndProcHandlerの中から元の処理も呼べるようにする
	m_prevWndProc = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHandler)));

	m_initialized = true;
}

//Init()の後片付け。Init()で確保したものを、作った時と逆の順番で解放する
void imguiApp::Uninit()
{
	if (!m_initialized)return;//初期化されていないなら何もしない

	ImGui_ImplDX11_Shutdown();//描画バックエンドの後片付け
	ImGui_ImplWin32_Shutdown();//入力バックエンドの後片付け
	ImGui::DestroyContext();//Init()で作ったImGuiのデータの入れ物を破棄する

	m_initialized = false;
}

//毎フレームの最初に呼ぶ関数。「これから今フレーム分のUIを組み立てます」という合図
//これを呼んだ後でないと、ImGui::Begin()などのUI構築関数は使えない
void imguiApp::NewFrame()
{
	if (!m_initialized)return;

	ImGui_ImplDX11_NewFrame();//描画側の1フレーム分の準備
	ImGui_ImplWin32_NewFrame();//入力側の1フレーム分の準備(マウス座標などの更新)
	ImGui::NewFrame();//ImGui本体に「新しいフレームが始まった」と伝える
}

//組み立てたImGuiのウィンドウを、実際に画面に描画する関数
//NewFrame()〜Render()の間にImGui::Begin~Endで組み立てた内容がここでGPUに送られる
void imguiApp::Render()
{
	if (!m_initialized)return;

	ImGui::Render();//ここまでに組み立てたUIを、描画用のデータ(頂点情報)に変換する
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());//その頂点情報を実際にDirectX11でGPUに描かせる
}

//カメラ調整用のデバッグウィンドウの中身を組み立てる関数
//座標(Position)・注視点(Target)・角度(Angle H/V)・距離(Distance)はお互いに連動していて、
//どれか1つを編集すると、残りが自動で計算し直される(軌道カメラの考え方)
void imguiApp::DrawCameraDebugWindow(const Vector3& currentPos, const Vector3& currentTarget)
{
	if (!m_initialized)return;

	//このBeginからEndの間だけ、項目間の縦の間隔を広げる(x=横の間隔, y=縦の間隔)
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, kCameraDebugItemSpacing);

	ImGui::Begin("Camera Debug");//"Camera Debug"という名前のウィンドウを開始する(ここから下がウィンドウの中身)

	//今実際に使われているカメラの座標・注視点を、文字として表示するだけ(この2行は編集できない)
	ImGui::Text("Current Pos    : (%.1f, %.1f, %.1f)", currentPos.x, currentPos.y, currentPos.z);
	ImGui::Text("Current Target : (%.1f, %.1f, %.1f)", currentTarget.x, currentTarget.y, currentTarget.z);

	//ボタンが押された瞬間だけtrueを返す。押されたら、実際のカメラの値をこのクラスの編集用変数にコピーする
	if (ImGui::Button("Sync From Current"))
	{
		m_camPos = currentPos;
		m_camTarget = currentTarget;
		RecalcAngleDistanceFromPos();//座標を書き換えたので、角度・距離の表示もそれに合わせて計算し直す
	}

	ImGui::Separator();//区切り線を1本描くだけ(見た目を整えるための飾り)
	if(ImGui::Checkbox("Override Camera", &m_overrideCamera))//チェックボックス。ON/OFFがそのままm_overrideCameraに入る
	{
		RecalcAngleDistanceFromPos();//座標を書き換えたので、角度・距離の表示もそれに合わせて計算し直す
	}
	//DragFloat3は「マウスでドラッグして値を変更できる、float3個(x,y,z)組の入力欄」を1個作る関数
	//戻り値は「今フレームで値が変わったかどうか」
	bool changed = false;

	// |= は論理和(OR)を計算して代入する演算子
	// ||には短絡評価(ショートサーキット)という性質があって
	// changed = changed || ImGui::DragFloat3("Target", &m_camTarget.x, 1.0f);
	//左側がtrueなら右側の関数が呼ばれなくなるー＞から困る
	//ので、この式にして、trueでもよばれるようにする
	changed |= ImGui::DragFloat3("Position", &m_camPos.x, kPosDragSpeed);
	changed |= ImGui::DragFloat3("Target", &m_camTarget.x, kPosDragSpeed);
	if (changed)
	{
		//座標側を直接いじったので、角度・距離の表示もズレないように計算し直す
		RecalcAngleDistanceFromPos();
	}

	//角度(度)と距離の入力欄。DragFloatの引数は(ラベル名, 対象の値, ドラッグ感度, 最小値, 最大値)
	bool changedByAngle = false;
	// |= は論理和(OR)を計算して代入する演算子
	//詳しくは上↑を見ること
	changedByAngle |= ImGui::DragFloat("Angle H (deg)", &m_angleHDeg, kAngleDragSpeed, kAngleHDegMin, kAngleHDegMax);
	changedByAngle |= ImGui::DragFloat("Angle V (deg)", &m_angleVDeg, kAngleDragSpeed, kAngleVDegMin, kAngleVDegMax);
	changedByAngle |= ImGui::DragFloat("Distance", &m_distance, kDistanceDragSpeed, kDistanceMin, kDistanceMax);
	if (changedByAngle)
	{
		//角度か距離を直接いじったので、座標側もそれに合わせて計算し直す
		RecalcPosFromAngleDistance();
	}

	ImGui::End();//"Camera Debug"ウィンドウの中身の記述はここまで、という合図
	ImGui::PopStyleVar();//8Pushした分をここで戻す
}

//角度・距離から座標を求める関数(注視点を中心にした球面上の1点を、三角関数を使って計算している)
//イメージ:注視点を原点として、水平角度・垂直角度の向きに、距離ぶんだけ離れた点を求めている
void imguiApp::RecalcPosFromAngleDistance()
{
	//度をラジアンに変換する(C++の三角関数sinf/cosfはラジアン単位でしか計算できないため)
	float radH = m_angleHDeg * kDegToRad;
	float radV = m_angleVDeg * kDegToRad;

	//「注視点から見てこの角度・距離の場所」までのずれ(オフセット)ベクトルを求める
	float x = m_distance * sinf(radH) * cosf(radV);
	float y = m_distance * sinf(radV);
	float z = -m_distance * cosf(radH) * cosf(radV);

	//注視点にオフセットを足したものが、カメラの座標になる
	m_camPos = m_camTarget + Vector3(x, y, z);
}

//座標・注視点から、角度・距離を逆算する関数(RecalcPosFromAngleDistanceの逆の計算)
void imguiApp::RecalcAngleDistanceFromPos()
{
	Vector3 diff = m_camPos - m_camTarget;//注視点から見た、カメラの相対的な位置(ベクトル)
	m_distance = diff.Magnitude();//そのベクトルの長さ = 注視点までの距離
	if (m_distance <= kMinDistanceEpsilon)return;//距離がほぼ0だと下の計算で0割りに近くなり不安定になるので中断する

	//asinf/atan2fで、ベクトルの向きから角度(ラジアン)を求め、kRadToDegを掛けて度に変換している
	m_angleVDeg = asinf(std::clamp(diff.y / m_distance, -1.0f, 1.0f)) * kRadToDeg;
	m_angleHDeg = atan2f(diff.x, -diff.z) * kRadToDeg;
}

//DxLibのウィンドウにマウス/キーボードのメッセージが届くたびにWindowsから呼ばれる関数
//(Init()内のSetWindowLongPtrによって、この関数が呼ばれるように設定されている)
LRESULT CALLBACK imguiApp::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//まずImGuiにメッセージを見せる。ImGuiが「これは自分が処理した」と判断したら(trueが返ったら)ここで終わり
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}

	//ImGuiが使わなかったメッセージは、DxLib本来のウィンドウプロシージャに渡す(ゲーム側の入力を壊さないため)
	auto& instance = imguiApp::GetInstance();
	if (instance.m_prevWndProc)
	{
		return CallWindowProc(instance.m_prevWndProc, hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
