#pragma once
#include "Camera.h"
class TitleCamera :
    public Camera
{
public:
	//タイトル演出のカット
	enum class Shot
	{
		Fixed,			//定点で映す
		FollowPlayer,	//プレイヤーを追従し続ける
		ZoomOut,
		Opening,
		Finish
	};

	TitleCamera();
	virtual ~TitleCamera();

	void Init()override;
	void Draw()override;
	void Update(Vector3 pos, Vector3 pos2 = Vector3()) override;
	void CameraSetting() override;

	void SetShot(Shot shot) { m_shot = shot; }
	Shot GetShot()const { return m_shot;}
private:
	Shot m_shot = Shot::Fixed;
	int m_count = 0;
	int m_count_zoom = 0;
};

