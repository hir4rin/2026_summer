#pragma once
#include <memory>

class Player;
class Weapon
{
public:
	Weapon(std::weak_ptr<Player> owner);
	virtual ~Weapon();



	void Update();
	void TitleUpdate();
	void Draw();
	
private:
	std::weak_ptr<Player> m_owner;//持ち主のプレイヤー
	int m_ownerHandle = -1;//持ち主のハンドル
	int m_modelHandle = -1;//剣のモデルのハンドル
	int slotIndex = 295;//装備しているスロットのインデックス

};

