#include "CameraHandler.h"
#include "Camera.h"

CameraHandler::CameraHandler()
{
}

CameraHandler::~CameraHandler()
{
}

void CameraHandler::ChangeCamera(std::shared_ptr<Camera> camera)
{
	if (m_cameras.empty())	// 最初は要素がないためpush_backで要素数を増やす
	{
		// 空っぽの場合には指定の要素をpushします。少なくとも一つは積まれている状態にする
		m_cameras.push_back(camera);
	}
	else
	{
		m_cameras.back() = camera;
		
		// この行の時点で元のカメラは自動的に削除されています
	}
}

void CameraHandler::PushCamera(std::shared_ptr<Camera> camera)
{
	// 新しいカメラを末尾に積みます
	m_cameras.push_back(camera);
}

void CameraHandler::PoPCamera()
{
	// 末尾のカメラを削除します
	m_cameras.pop_back();
}

void CameraHandler::Update()
{
	if (m_cameras.empty())return;
	// 末尾の要素に対してのみUpdateする
	m_cameras.back()->Update(Vector3(), Vector3());
}
void CameraHandler::Draw()
{
	if (m_cameras.empty())return;
	// 末尾の要素に対してのみDrawする
	m_cameras.back()->Draw();
}
