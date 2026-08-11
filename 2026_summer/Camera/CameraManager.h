#pragma once
#include <memory>
#include <list>
#include <vector>
#include "../Math/Vector3.h"
#include "Camera.h"

class Input;
class Player;
class EnemyBase;
class MainCamera;
class Stage;

struct CameraContext
{
	std::weak_ptr<Player> m_player;
	std::weak_ptr<EnemyBase> m_targetEnemy;
	//std::weak_ptr<EnemyBase> m_targetEnemyNoLockOn;
	bool m_isUltimate = false;
};

class CameraManager : public std::enable_shared_from_this<CameraManager>
{
public:
	CameraManager();
	virtual ~CameraManager();
	//カメラを登録
	void EntryCamera(std::shared_ptr<Camera> camera);
	//カメラを削除
	void RemoveCamera(std::shared_ptr<Camera> camera);

	//Playerのweak_ptrを渡す
	void Init(std::weak_ptr<Player> player, std::weak_ptr<Stage> stage = {});
	/// <summary>
	/// 
	/// </summary>
	/// <param name="pos">targetの座標</param>
	/// <param name="pos2">補助的な座標</param>
	void Update(Vector3 pos, Vector3 pos2 = Vector3());
	void Draw();

	/// <summary>
	/// レンダーターゲットごとに変わってしまうので、カメラの設定を反映させる
	/// </summary>
	void ApplyCameraSettings();

	std::shared_ptr<Camera> GetHighestPriorityCamera()const { return highestPriorityCamera; }
	//ここですること
	//カメラの更新//priorityの高いカメラのUpdateを呼ぶ
	//lerpはどうしよう//補完用のクラスを作る
	//カメラを切り替えた後、何フレーム化したら元に戻る処理を作る//各自でやらせたほうがよさそう

	//指定したTypeのカメラのpriorityを一つ上げる
	void SetNextCameraPriority(Camera::Type type,bool isLerp = false,bool isSlerp = false);

	//RefWeakptr用
	void SetWeakRef(std::weak_ptr<Player> m_player, std::weak_ptr<EnemyBase> m_enemy = {});
	void SetWeakTargetEnemy(int id);//idで指定したEnemyをターゲットにする
	//mainCameraのゲット
	std::shared_ptr<MainCamera> GetMainCamera() { return m_mainCamera; }
	//ターゲットのEnemyを取得する
	std::shared_ptr<EnemyBase> GetTargetEnemy()const { return m_context->m_targetEnemy.lock(); }

	//PlayerCameraに角度をセットさせる関数
	void SetPlayerCameraAngle(float angleH, float angleV);

	//タイトル画面かどうかをセットする//trueの場合、TitleCamera以外の更新・優先度争いを行わない
	void SetIsTitle(bool isTitle) { m_isTitle = isTitle; }

private:
	void SetUpMainCamera();//priorityが最も高いカメラの情報をMainCameraに反映させる

private:
	std::list<std::shared_ptr<Camera>> m_cameras;//カメラのリスト
	std::shared_ptr<Camera> highestPriorityCamera;//priorityが最も高いカメラ//Updateを呼ぶときに使う
	std::vector<std::shared_ptr<Camera>> m_weakRefCameras;
	//カメラ
	std::shared_ptr<MainCamera> m_mainCamera;
	std::shared_ptr<Camera> m_playerCamera;
	std::shared_ptr<Camera> m_movieCamera;
	std::shared_ptr<Camera> m_ultCamera;
	std::shared_ptr<Camera> m_LockOnCamera;
	std::shared_ptr<Camera> m_TitleCamera;
	//必要な情報
	std::shared_ptr<CameraContext> m_context;
	//不本意だがいまはとりあえずここにかく
	bool m_isUltimating = false;
	bool m_isPrevUltimating = false;
	bool m_isTitle = false;//タイトル画面かどうか//trueの場合、TitleCamera以外の更新・優先度争いを行わない

};

