#pragma once
#include <vector>
#include <memory>

class UIBase;

class UIManager
{
public:
	UIManager();
	~UIManager();

	void Init();
	void Update();
	void Draw();

	void AddUI(std::shared_ptr<UIBase> ui) { m_uiLists.push_back(ui); }
private:
	std::vector<std::shared_ptr<UIBase>> m_uiLists;

};

