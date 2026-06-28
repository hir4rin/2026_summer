#pragma once
#include <memory>
#include <list>
#include <vector>
#include "../Math/Vector3.h"
#include "Camera.h"

class Input;


class CameraManager
{
public:
	CameraManager();
	virtual ~CameraManager();
	//カメラを登録
	void EntryCamera(std::shared_ptr<Camera> camera);
	//カメラを削除
	void RemoveCamera(std::shared_ptr<Camera> camera);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="pos">targetの座標</param>
	/// <param name="pos2">補助的な座標</param>
	void Update(Vector3 pos, Vector3 pos2 = Vector3());
	/// <summary>
	/// レンダーターゲットごとに変わってしまうので、カメラの設定を反映させる
	/// </summary>
	void ApplyCameraSettings();

	std::shared_ptr<Camera> GetHighestPriorityCamera()const { return highestPriorityCamera; }
	//ここですること
	//カメラの更新//priorityの高いカメラのUpdateを呼ぶ
	//lerpはどうしよう//補完用のクラスを作る
	//カメラを切り替えた後、何フレーム化したら元に戻る処理を作る//各自でやらせたほうがよさそう

	//priorityをすべて変更する関数があれば便利かも//enumClassなどで、分けて、やるのがよさげ
	void SetAllCameraPriority(Camera::Type type);
private:
	std::list<std::shared_ptr<Camera>> m_cameras;//カメラのリスト
	std::shared_ptr<Camera> highestPriorityCamera;//priorityが最も高いカメラ//Updateを呼ぶときに使う
	//カメラ
	std::shared_ptr<Camera> m_playerCamera;
	std::shared_ptr<Camera> m_movieCamera;

};

