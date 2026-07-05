#include "Weapon.h"
#include "DxLib.h"
#include "Base/Player.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4x4.h"

Weapon::Weapon(std::weak_ptr<Player> owner)
	: m_owner(owner)
{
	auto player = m_owner.lock();
	if (!player)return;

	m_modelHandle = MV1LoadModel("data/Player/Weapon/katana_blend.mv1");
}

Weapon::~Weapon()
{
}

void Weapon::Update()
{
	auto player = m_owner.lock();
	if (!player)return;
	//モデルフレームのローカルワールド行列を取得
	MATRIX mat = MV1GetFrameLocalWorldMatrix(player->GetModelHandle(), slotIndex);//モデルフレームのローカルワールド行列を取得

	////武器の位置を取得
	//Vector3 weaponPos = MV1GetFramePosition(m_ownerHandle, slotIndex);

	//剣先を取得する簡単なやり方があるらしい

	//MATRIX transmat = MGetTranslate(weaponPos.ToDxLibVector());
	//90度回転させる
	MATRIX rotmat = MGetRotY(DX_PI_F);//回転行列を作成する//90度回転させる
	mat = MMult(rotmat, mat);//回転行列を掛ける//90度回転させる

	MATRIX scale = MGetScale(VGet(0.6f, 0.6f, 0.6f));//スケーリング行列を作成する//モデルの大きさを半分にする

	mat = MMult(scale, mat);//スケーリング行列を掛ける//モデルの大きさを半分にする

	//モデルにマトリクスをセット
	MV1SetMatrix(m_modelHandle, mat);
	////一旦0で初期化して確認
	//Vector3 zero = Vector3(0.0f, 0.0f, 0.0f);
	//MV1SetPosition(m_modelHandle, zero.ToDxLibVector());


}

void Weapon::TitleUpdate()
{
	//モデルフレームのローカルワールド行列を取得
	MATRIX mat = MV1GetFrameLocalWorldMatrix(m_ownerHandle, slotIndex);//モデルフレームのローカルワールド行列を取得
	//MATRIX transmat = MGetTranslate(weaponPos.ToDxLibVector());
	//90度回転させる
	//MATRIX rotmat = MGetRotY(DX_PI_F / 2.0f);//回転行列を作成する//90度回転させる
	//mat = MMult(rotmat, mat);//回転行列を掛ける//90度回転させる

	MATRIX scale = MGetScale(VGet(0.6f, 0.6f, 0.6f));//スケーリング行列を作成する//モデルの大きさを半分にする

	mat = MMult(scale, mat);//スケーリング行列を掛ける//モデルの大きさを半分にする

	//モデルにマトリクスをセット
	MV1SetMatrix(m_modelHandle, mat);

}

void Weapon::Draw()
{
	// 実際の位置を取得して表示
	VECTOR pos = MV1GetPosition(m_modelHandle);

	MV1DrawModel(m_modelHandle);

	
}
