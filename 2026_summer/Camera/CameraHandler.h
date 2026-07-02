#pragma once
#include <memory>
#include <list>

class Camera;

class CameraHandler
{
public:
	CameraHandler();
	~CameraHandler();
	
	/// <summary>
	/// カメラを切り替える//初期化される
	/// </summary>
	/// <param name="camera"></param>
	void ChangeCamera(std::shared_ptr<Camera> camera);

	/// <summary>
	/// シーンを新しく積む//初期化されない
	/// </summary>
	/// <param name="camera"></param>
	void PushCamera(std::shared_ptr<Camera> camera);

	/// <summary>
	/// 最期に積んだシーンを外す
	/// </summary>
	void PoPCamera();

	//内部に持っているカメラのUpdateを呼ぶ
	void Update();

	void Draw();
private:
	std::list<std::shared_ptr<Camera>> m_cameras;
};

