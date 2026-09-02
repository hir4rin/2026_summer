#pragma once
#include "Scene.h"
#include "GameResult.h"
#include <memory>

class CameraManager;
class Player;

//ステージクリア結果を、PauseSceneのようにGameSceneの上に重ねて表示するポップアップシーン
class StageClearPoPScene :
    public Scene
{
public:
    StageClearPoPScene(SceneController& controller, const GameResult& result);
    virtual ~StageClearPoPScene();

    void Update() override;
    void FadeInUpdate() override;
    void NormalUpdate() override;
    void FadeOutUpdate() override;

    void Draw() override;
    void FadeInDraw() override;
    void NormalDraw() override;
	void FadeOutDraw() override;
private:
	//リザルトの表示項目
	enum class ResultItem : int
	{
		ClearTime = 0,
		Damage,
		Combo,
		DamageTaken,
		Size,
	};
	//リザルト演出の進行状況
	enum class Phase
	{
		Counting,//1項目ずつカウントアップして表示している最中
		Rank,//ランクを表示している最中
		Done,//すべて表示し終わり、入力待ち
	};

	float GetItemTarget(ResultItem item)const;//項目の最終的な表示値を返す
	void DrawItem(ResultItem item, int y)const;//項目を1行描画する
	char CalcRank()const;//クリアタイムからランクを算出する

private:
	std::shared_ptr<CameraManager> m_cameraManager;//結果画面表示用に自前で持つカメラ(ResultCameraのみ使用)
	std::shared_ptr<Player> m_player;//結果画面表示用に自前で持つプレイヤー(GameSceneの実プレイヤーとは別、赤くして表示する)

	GameResult m_result;//集計されたリザルト

	Phase m_phase = Phase::Counting;
	int m_itemIndex = 0;//現在カウントアップ中の項目(ResultItem)
	int m_itemFrame = 0;//現在の項目のカウント開始からのフレーム数
	float m_displayValues[static_cast<int>(ResultItem::Size)] = {};//現在表示中の値(カウントアップ中は途中の値になる)

	int m_rankFrame = 0;//ランク表示開始からのフレーム数
	char m_rank = 'C';//算出されたランク

	int m_fadeCount = 0;//フェードイン・フェードアウトの経過フレーム

	int m_pressButtonBlinkFrame = 0;//"Press A Button"点滅演出の経過フレーム

};
