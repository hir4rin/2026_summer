#pragma once
#include <memory>
#include "Scene.h"
#include "../UI/UIManager.h"
#include <d3dcompiler.h>
#include <string>


class CameraManager;
class Stage;
class SkyBox;
class Player;
class EnemySwordman;
class EnemyManager;

//シェーダー
struct UltParam
{
	float ultAmount = 0.0f;//黒さの割合
	float padding[3] = {};
};	




class GameScene : public Scene
{
public:
		GameScene(SceneController& controller);
		virtual ~GameScene();
		void Update() override;
		void FadeInUpdate() override;
		void NormalUpdate() override;
		void FadeOutUpdate() override;

		void Draw() override;
		void FadeInDraw() override;
		void NormalDraw() override;
		void FadeOutDraw() override;
private:
	void DrawGrid();
	//ロックオンカメラのボタンを押すチェック処理
	void CheckLockOnCamera();
	//ロックオン中のカメラの入力処理
	void LockOnCameraInput();
	//ロックオンしている敵が死んだら、次の敵に切り替える
	void CheckLockOnCameraEnemyDead();

	int LoadVertexShaderWithMacro(const std::string& filePath, std::vector<D3D_SHADER_MACRO>& macros);
private:
	

private:
	std::shared_ptr<CameraManager> m_cameraManager;
	std::shared_ptr<Player> m_player;
	std::shared_ptr<EnemySwordman> m_enemySwordman;
	std::shared_ptr<EnemyManager> m_enemyManager;
	std::shared_ptr<Stage> m_stage;
	std::shared_ptr<SkyBox> m_skyBox;

	std::shared_ptr<UIManager> m_uiManager;

	int m_gHandle1 = -1;
	int m_gHandle2 = -1;
	int m_gHandle3 = -1;

	int m_efAreaHandle = -1;//壁エフェクトのハンドル
	int m_efAreaPlayingHandle = -1;//再生中

	//シェーダー用
	int m_RT1 = -1;//一部UI描画用
	int m_RT2 = -1;//必殺技時のエフェクト用

	int m_ultShaderHandle = -1;
	UltParam* m_ultCBuff = nullptr;

	int m_enemyPSH = -1;
	int m_enemyVSH = -1;

	int m_playFrameCount = 0;//リザルトのクリアタイム集計用//プレイ開始からのフレーム数


};

