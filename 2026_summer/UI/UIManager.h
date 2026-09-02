#pragma once
#include <vector>
#include <memory>

class UIBase;
class ComboHUD;

class UIManager
{
public:
	UIManager();
	~UIManager();

	void Init();
	void Update();
	void Draw();

	void AddUI(std::shared_ptr<UIBase> ui) { m_uiLists.push_back(ui); }

	void ToggleComboUI();//コンボUIの表示のオン/オフを切り替える
private:
	std::vector<std::shared_ptr<UIBase>> m_uiLists;

	std::shared_ptr<ComboHUD> m_comboHUD;//表示のオン/オフを切り替えるために個別に持っておく

};

