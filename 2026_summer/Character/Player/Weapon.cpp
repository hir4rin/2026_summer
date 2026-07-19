#include "Weapon.h"
#include "DxLib.h"
#include "Base/Player.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4x4.h"
#include "../System.h"

Weapon::Weapon(std::weak_ptr<Player> owner)
	: m_owner(owner)
{
	auto player = m_owner.lock();
	if (!player)return;

	m_modelHandle = System::GetInstance().GetHandle(AsyncData::PlayerWeaponModel);
}

Weapon::~Weapon()
{
}

void Weapon::Update()
{
	auto player = m_owner.lock();
	if (!player)return;
	//モデルフレームのローカルワールド行列を取得
	//プレイヤーの状態によってモデルを変える
	int modelHandle = -1;
	int currentModelHandle = player->m_anim.GetModelHandleForCheck();
	if (currentModelHandle == player->m_modelHandle)
	{
		modelHandle = player->m_modelHandle;
	}
	else
	{
		modelHandle = player->m_attackModelHandle;
	}
	

	MATRIX mat = MV1GetFrameLocalWorldMatrix(modelHandle, slotIndex);//モデルフレームのローカルワールド行列を取得

	////武器の位置を取得
	//Vector3 weaponPos = MV1GetFramePosition(m_ownerHandle, slotIndex);

	//剣先を取得する簡単なやり方があるらしい

	//MATRIX transmat = MGetTranslate(weaponPos.ToDxLibVector());
	//90度回転させる
	MATRIX rotmat = MGetRotY(DX_PI_F / 2);//回転行列を作成する//90度回転させる
	MATRIX rotXmat = MGetRotX(DX_PI_F / 2);//回転行列を作成する//90度回転させる
	//回転の合成
	rotmat = MMult(rotXmat, rotmat);//回転行列を掛ける//90度回転させる

	//下方向に移動
	MATRIX transmat = MGetTranslate(VGet(45.0f, 0.0f, 0.0f));//移動行列を作成する//下方向に移動
	rotmat = MMult(transmat, rotmat);//移動行列を掛ける//下方向に移動
	mat = MMult(rotmat, mat);//回転行列を掛ける//90度回転させる


	MATRIX scale = MGetScale(VGet(0.3f, 0.3f, 0.3f));//スケーリング行列を作成する//モデルの大きさを半分にする

	mat = MMult(scale, mat);//スケーリング行列を掛ける//モデルの大きさを半分にする

	//モデルにマトリクスをセット
	MV1SetMatrix(m_modelHandle, mat);
	////一旦0で初期化して確認
	//Vector3 zero = Vector3(0.0f, 0.0f, 0.0f);
	//MV1SetPosition(m_modelHandle, zero.ToDxLibVector());


}



void Weapon::Draw()
{
	// 実際の位置を取得して表示
	VECTOR pos = MV1GetPosition(m_modelHandle);

	MV1DrawModel(m_modelHandle);

	
}
