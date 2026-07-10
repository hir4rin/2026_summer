#include "System.h"


void System::Update()
{
	//if (m_frameCount < 0)return;

	m_frameCount--;

	m_ultCount -= 1.0f * timeScale;

	if (m_frameCount <= 0)
	{
		timeScale = 1.0f;//時間のスケールを元に戻す
		m_frameCount = -1;//フレームカウントを0にする
	}
	if(m_ultCount <= 0)
	{
		m_ultCount = -1;
		m_isUltimating = false;
	}

}
