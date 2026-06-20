#include "CharacterBase.h"
#include <assert.h>
#include "../Math/Matrix4x4.h"

CharacterBase::CharacterBase()
{
}

CharacterBase::~CharacterBase()
{
}

void CharacterBase::InitHitCol(std::weak_ptr<CharacterBase> owner)
{
	m_hitCol = std::make_unique<HitCol>(owner);
}

void CharacterBase::UpdateAngleAndPos()
{
	float targetAngle = 0.0f;//目標の角度
	if (m_targetVec.Magnitude() > 0.0f)//最初の入力されないとき以外、ここを通り、モデルの向きを変える
	{
		//モデルの移動方向にモデルの方向を近づける
		targetAngle = atan2f(m_targetVec.x, m_targetVec.z);//移動ベクトルのx成分とz成分から、プレイヤーの向きたい方向の角度を求める
		// Y軸回転行列を作成する//この工程は毎フレーム、原点からモデルの位置に移動してから、回転する行列を作成している
		//180度ずれてたので、回転角度を180度ずらす
		/* m_rotAngle = targetAngle - DX_PI_F;*/
		 //角度の差分を計算//回転角度を-90から90にするため(最短経路を選択)
		float difference = targetAngle - m_rotAngleY - DX_PI_F;
		while (difference > DX_PI_F) difference -= 2.0f * DX_PI_F;
		while (difference < -DX_PI_F) difference += 2.0f * DX_PI_F;
		//targetAngle + DX_PI_F
		m_rotAngleY += difference * 0.1f;//回転角度を少しずつ目標の角度に近づける//ほぼlerp
	}
	//モデルは、座標の位置のcenter分下で表示

	Matrix4x4 rotY = Matrix4x4::MakeRotationY(m_rotAngleY);
	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);

	Matrix4x4 mtx = trans * rotY;
	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));
}

void CharacterBase::ApplyPos()
{
	//座標の更新//m_knockBackVelを加算する
	m_pos += m_vel;

	//モデルの座標を更新する
	UpdateAngleAndPos();

	//お試し
	//Vector3 totalVel;
	//std::vector<std::string> RemoveList;
	//for (auto& [name, velInfo] : m_velocities)
	//{
	//	totalVel += velInfo.vel;
	//	if (velInfo.decayRate == 0.0f)//1フレームで削除する場合
	//	{
	//		RemoveList.push_back(name);
	//	}
	//	else
	//	{
	//		velInfo.vel *= velInfo.decayRate;//減衰
	//		if(velInfo.vel.Magnitude() < 0.5f)//速度が小さくなったら削除する
	//		{
	//			RemoveList.push_back(name);
	//		}
	//	}
	//}
	//for (const auto& name : RemoveList)
	//{
	//	m_velocities.erase(name);
	//}
	//m_pos += totalVel;
	//UpdateAngleAndPos();
}


const std::string& CharacterBase::GetAnimName(const std::string& key) const
{
	auto it = m_animNames.find(key);
	assert(it != m_animNames.end() && "指定されたキーが見つかりませんでした");
	return it->second;

}