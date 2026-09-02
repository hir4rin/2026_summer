#pragma once
#include <string>
#include <unordered_map>

namespace
{
	//マスター音量の初期値(255が最大)
	constexpr int kDebugBgmMasterVolumeDefault = 80;
	constexpr int kDebugSEMasterVolumeDefault = 100;
	constexpr int kBgmMasterVolumeDefault = 90;
	constexpr int kSEMasterVolumeDefault = 100;
}

struct SeSlot
{
	int handle;
};

class SoundManager
{
public:

	void Init();
	void Update();

	void PlaySE(const std::string& name);
	void PlayBgm(const std::string& name);
	void PlaySELoop(const std::string& name);

	void StopBgm();
	void StopSE(const std::string& name);
	//セット
	void SetBgmVolume(int volume);//0～255
	void SetSEVolume(int volume);//0～255

	//音量設定用
	int GetBgmVolume() { return m_masterVolume; }
	int GetSEVolume() { return m_masterVolume2; }

private:
	std::unordered_map<std::string, int> m_sounds;
	std::unordered_map<std::string, int>m_soundVolumes;//名前と音量を紐づける
	std::unordered_map<std::string, int>m_seVolumes;//名前と音量を紐づける
	//内部的なもの
#ifdef _DEBUG
	int m_masterVolume = kDebugBgmMasterVolumeDefault;//255が最大
	int m_masterVolume2 = kDebugSEMasterVolumeDefault;//255が最大
#else
	int m_masterVolume = kBgmMasterVolumeDefault;//255が最大
	int m_masterVolume2 = kSEMasterVolumeDefault;//255が最大
#endif
	//見た目用
	int m_currentBgm = -1;
	
	//3重再生用スロット
	std::vector<SeSlot> m_seSlots;
};


