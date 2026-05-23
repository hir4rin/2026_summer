#pragma once
#include "Camera.h"
class Movie1Camera :
    public Camera
{
public:
    Movie1Camera();
    virtual ~Movie1Camera();
    void Init()override;
    void Update(Vector3 pos,Vector3 pos2 = Vector3()) override;
	void FixCameraPos() override;

	

	void CountDiePriority();
	void SetMovieStartFrame(float frame) { m_invisibleFrame = frame; }//カメラマネージャーからセットする
private:
	float m_invisibleFrame;//カメラを切り替えてから、何フレームか経ったかを数える//一定のフレーム数までは、ムービーカメラを優先するようにするためのもの
	Vector3 m_nextpos;//次のカメラの位置//カメラの位置を滑らかに移動させるためのもの
};

