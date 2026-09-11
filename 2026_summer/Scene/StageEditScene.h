#pragma once
#include "Scene.h"
#include "../Math/Vector3.h"
#include "../Stage/StageObjectData.h"
#include <vector>
#include <memory>

class StageObject;

/// <summary>
/// ステージ制作モード(BOXの配置・調整・CSV保存)とプレイ確認モードを
/// bool(m_isEditMode)で切り替えられる、ステージ編集専用シーン
/// </summary>
class StageEditScene : public Scene
{
public:
	StageEditScene(SceneController& controller);
	virtual ~StageEditScene();

	void Update() override;
	void FadeInUpdate() override;
	void NormalUpdate() override;
	void FadeOutUpdate() override;

	void Draw() override;
	void FadeInDraw() override;
	void NormalDraw() override;
	void FadeOutDraw() override;

private:
	/// <summary>ステージ制作モード中、左クリック長押しでBOXを生成するためのドラッグ入力を処理する</summary>
	void UpdateEditInput();

	/// <summary>ImGuiでステージ編集用のウィンドウを組み立てる</summary>
	void DrawEditorUI();

	/// <summary>現在のm_stageNumberのCSVを読み込み、配置済みBOXを作り直す</summary>
	void LoadStage();
	/// <summary>現在配置しているBOXを、m_stageNumberのCSVとして保存する</summary>
	void SaveStage();
	/// <summary>配置済みのBOXを全て破棄する(CollisionManagerからも解除する)</summary>
	void ClearPlacedObjects();

	/// <summary>マウスカーソルの画面座標から、planeYの高さの水平面上のワールド座標を求める</summary>
	Vector3 GetMouseGroundPos(float planeY) const;

	/// <summary>ドラッグ中のBOXのプレビュー(まだ確定していない枠線)を描画する</summary>
	void DrawDraggingPreview() const;

private:
	std::vector<std::shared_ptr<StageObject>> m_placedObjects;	//配置済みのBOX
	int m_selectedIndex = -1;										//ImGuiで選択中のBOXのインデックス(-1で未選択)

	bool m_isEditMode = true;		//true:ステージ制作モード(BOX配置可能) / false:プレイ確認モード
	int m_stageNumber = 1;			//現在編集中のステージ番号(CSVファイル名に使う)

	bool m_isDragging = false;		//左クリックを押し始めてから離すまでの間true
	bool m_wasMousePressed = false;//前フレームで左ボタンが押されていたか(押した瞬間の検出用)
	Vector3 m_dragStartPos;			//ドラッグ開始時の地面座標
	Vector3 m_dragCurrentPos;		//ドラッグ中の現在の地面座標

	float m_createHeight = 100.0f;	//次に生成するBOXの高さ(Y方向の全長)
	float m_createBaseY = 0.0f;	//次に生成するBOXの中心Y座標

private:
	static constexpr float kGroundPlaneY = 0.0f;		//BOXの footprint をドラッグする基準の地面の高さ
	static constexpr float kMinFootprintSize = 5.0f;	//これより小さいドラッグ量は誤クリックとみなして無視する
};
