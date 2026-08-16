#include "Titlemascot.h"
#include "../../Math/Matrix4x4.h"
#include "../System.h"

namespace
{
	constexpr float kMascotCenter = 50.0f;//当たり判定の中心点までのy軸の距離
	constexpr float kMascotRadius = 40.0f;//当たり判定の半径
}

Titlemascot::Titlemascot(Vector3 pos)
{
	m_pos = pos;
	m_rotAngleY = 90.0f;
	m_modelHandle = MV1DuplicateModel(System::GetInstance().GetHandle(AsyncData::MascotModel));


	//モデルの初期位置を設定する
	Matrix4x4 rotY = Matrix4x4::MakeRotationY(-m_rotAngleY);
	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);
	Matrix4x4 mtx = trans * rotY;
	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));
	m_anim.Init(m_modelHandle, "mixamo.com",true);
}

Titlemascot::~Titlemascot()
{
	MV1DeleteModel(m_modelHandle);
}

void Titlemascot::Init()
{
	//IDの取得
	SetID();
	//当たり判定の初期化//中心点、オフセット、半径、当たり判定のタイプ、タグ、当たり判定が有効かどうか
	ColInit(m_pos, Vector3(0, kMascotCenter, 0), kMascotRadius, ColliderType::Sphere, Tags::Mascot, false,true);
}

void Titlemascot::Update()
{
	switch (m_state)
	{
	case State::First:

		break;
	case State::Kirimomi:
		m_vel += Vector3(0, 3, -5);
		break;
	}
	m_anim.Update();
	m_pos += m_vel;
	CharacterBase::ApplyPos();
}

void Titlemascot::Draw()
{
	MV1DrawModel(m_modelHandle);
}

void Titlemascot::OnCollision(Collider& other)
{
}

void Titlemascot::OnDamage(Collider& other, AttackData& data)
{
}
void Titlemascot::ApplyPos()
{
	
}
