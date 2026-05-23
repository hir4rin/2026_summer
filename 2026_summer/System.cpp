#include "System.h"


void System::Update()
{
	if (m_frameCount < 0)return;

	m_frameCount--;

	if (m_frameCount <= 0)
	{
		timeScale = 1.0f;//時間のスケールを元に戻す
		m_frameCount = -1;//フレームカウントを0にする
	}

}
