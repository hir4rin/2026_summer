#include "System.h"
#include "DxLib.h"
#include "EffekseerForDXLib.h"


void System::SetTerminate()
{
	MV1DeleteModel(m_asyncHandles[AsyncData::PlayerModel]);
	MV1DeleteModel(m_asyncHandles[AsyncData::PlayerAttackModel]);
	MV1DeleteModel(m_asyncHandles[AsyncData::PlayerWeaponModel]);
	MV1DeleteModel(m_asyncHandles[AsyncData::PlayerWingModel]);
	DeleteEffekseerEffect(m_asyncHandles[AsyncData::PlayerEffectSkill]);
	//enemy
	MV1DeleteModel(m_asyncHandles[AsyncData::EnemyModel]);
	//stage
	MV1DeleteModel(m_asyncHandles[AsyncData::StageModel]);
	MV1DeleteModel(m_asyncHandles[AsyncData::StageModelCollider]);

	m_asyncHandles[AsyncData::StageModel] = MV1LoadModel("data/Stage_Graphic/Stage.mv1");
	m_asyncHandles[AsyncData::StageModelCollider] = MV1LoadModel("data/Stage_Graphic/coll.mv1");
}

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
