#pragma once
#include "../CharacterBase.h"

class Titlemascot : public CharacterBase
{
public:
	enum class State
	{
		First,
		Kirimomi,
	};

public:
	Titlemascot(Vector3 pos);
	virtual ~Titlemascot();

	void Init();
	void Update();
	void Draw();

	void OnCollision(Collider& other) override;
	void OnDamage(Collider& other, AttackData& data) override;
	void ApplyPos()override;

	void SetState(State state) { m_state = state; }

private:
	State m_state = State::First;
};
