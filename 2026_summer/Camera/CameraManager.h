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
	void Init(std::weak_ptr<Player> player,std::weak_ptr<Stage> stage);
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
	//mainCameraのゲット
	std::shared_ptr<MainCamera> GetMainCamera() { return m_mainCamera; }

	//PlayerCameraに角度をセットさせる関数
	void SetPlayerCameraAngle(float angleH, float angleV);



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
	//必要な情報
	std::shared_ptr<CameraContext> m_context;
	//不本意だがいまはとりあえずここにかく
	bool m_isUltimating = false;
	bool m_isPrevUltimating = false;

};

