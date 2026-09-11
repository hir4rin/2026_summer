#include "StageObject.h"

StageObject::StageObject()
{
}

StageObject::~StageObject()
{
}

void StageObject::Init(const Vector3& pos, const Vector3& halfExtents)
{
	//radiusは使わないので0で初期化し、直後にBoxの半分の大きさを設定し直す
	ColInit(pos, Vector3(), 0.0f, ColliderType::Box, Tags::StaticObject, true);
	SetBoxHalfExtents(halfExtents);
}

void StageObject::OnCollision(Collider& other)
{
	//何もしない(押し戻しはCollisionManager側の共通処理に任せる)
}

void StageObject::ApplyPos()
{
	//ステージ制作モードでSetTransformされる以外、座標は動かないため何もしない
}

void StageObject::Draw() const
{
	DebugDraw();
}

void StageObject::SetTransform(const Vector3& pos, const Vector3& halfExtents)
{
	m_pos = pos;
	SetBoxHalfExtents(halfExtents);
}
