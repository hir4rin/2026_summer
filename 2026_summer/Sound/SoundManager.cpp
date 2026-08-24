#include "SoundManager.h"
#include "DxLib.h"
#include <cassert>


namespace
{
    //音量変換
    constexpr int kVolumeMax = 255;
    constexpr int kDisplayMax = 100;

}

void SoundManager::Init()
{
	//BGmの読み込み
    m_sounds["bgm"] = LoadSoundMem("data/Sound/new/TitleBGM.mp3");
    m_sounds["GameEnding"] = LoadSoundMem("data/Sound/new/GameEnding.mp3");
    m_sounds["GameBGM"] = LoadSoundMem("data/Sound/new/GamePlay.ogg");
    m_sounds["ResultBGM"] = LoadSoundMem("data/Sound/new/ResultBGM.ogg");
    
	assert(m_sounds["bgm"] >= 0);
	//音量の設定
    m_soundVolumes["bgm"] = m_masterVolume*0.9f;
    m_soundVolumes["GameEnding"] = m_masterVolume * 1.2f;
    m_soundVolumes["GameBGM"] = m_masterVolume *0.8f;
    m_soundVolumes["ResultBGM"] = m_masterVolume;



	//SEの読み込み
	//環境音

	//攻撃音
	m_sounds["airSlash"] = LoadSoundMem("data/Sound/SE/airSlash.mp3");
	m_sounds["LightAttackSE"] = LoadSoundMem("data/Sound/SE/LightAttackSE.mp3");
	m_sounds["HeavyAttackSE"] = LoadSoundMem("data/Sound/SE/HeavyAttackSE.mp3");
	m_sounds["shotArrow"] = LoadSoundMem("data/Sound/SE/shotArrow.mp3");
	m_sounds["hitArrow"] = LoadSoundMem("data/Sound/SE/hitArrow.mp3");
	m_sounds["hitOut"] = LoadSoundMem("data/Sound/SE/hitOut.mp3");
	m_sounds["hitSE"] = LoadSoundMem("data/Sound/SE/hitSE.mp3");
    //playerからでる音
	m_sounds["walkSE"] = LoadSoundMem("data/Sound/new/walkSE.mp3");
	m_sounds["gameWalkSE"] = LoadSoundMem("data/Sound/new/GameWalkSE.mp3");
	m_sounds["avoidSE"] = LoadSoundMem("data/Sound/new/avoidSE.mp3");
	//環境音
	m_sounds["rulet"] = LoadSoundMem("data/Sound/new/rulet.mp3");
	m_sounds["ruletSlow"] = LoadSoundMem("data/Sound/new/ruletSlow.mp3");
	m_sounds["rank"] = LoadSoundMem("data/Sound/new/rank.mp3");
	m_sounds["countDown"] = LoadSoundMem("data/Sound/new/CountDown.mp3");
	m_sounds["select"] = LoadSoundMem("data/Sound/SE/ok.mp3");
    //音量の設定
	m_seVolumes["airSlash"] = m_masterVolume2 * 1.0f;
	m_seVolumes["LightAttackSE"] = m_masterVolume2 * 1.0f;
	m_seVolumes["HeavyAttackSE"] = m_masterVolume2 * 1.0f;
	m_seVolumes["shotArrow"] = m_masterVolume2 * 1.0f;
	m_seVolumes["hitArrow"] = m_masterVolume2 * 1.0f;
    m_seVolumes["hitOut"] = m_masterVolume2 * 1.0f;
  	m_seVolumes["hitSE"] = m_masterVolume2 * 1.0f;
	m_seVolumes["walkSE"] = m_masterVolume2 * 2.2f;
	m_seVolumes["gameWalkSE"] = m_masterVolume2 * 1.5f;
	m_seVolumes["avoidSE"] = m_masterVolume2 * 1.0f;
	m_seVolumes["rulet"] = m_masterVolume2 * 1.5f;
	m_seVolumes["ruletSlow"] = m_masterVolume2 * 1.5f;
	m_seVolumes["rank"] = m_masterVolume2 * 1.5f;
	m_seVolumes["countDown"] = m_masterVolume2 * 1.0f;
	m_seVolumes["select"] = m_masterVolume2 * 1.0f;

    //敵の音
	// サウンド名のリスト
    const std::vector<std::string> soundNames = {
        "bgm",
		"GameEnding",
        "bgmStageScene",
        "bgmClearScene",
        "ResultBGM"
    };
	//SEのリスト
    const std::vector<std::string> seNames = {
     
        "airSlash",
		"LightAttackSE",
		"HeavyAttackSE",
        "shotArrow",
        "hitArrow",
        "hitOut",
        "hitSE",
		"walkSE",
		"gameWalkSE",
		"avoidSE",
        "rulet",
		"ruletSlow",
		"rank",
		"countDown",
		"select"
	};
	
	//BGmの音量設定
  // BGM音量
    for (const auto& key : soundNames)
    {
        ChangeVolumeSoundMem(m_soundVolumes[key], m_sounds[key]);
    }
	//SEの音量設定
    for (const auto& key : seNames)
    {
        ChangeVolumeSoundMem(m_seVolumes[key], m_sounds[key]);
    }
	//3重再生用スロットの初期化
    for (int i = 0; i < 3; ++i)
    {
        int h = LoadSoundMem("data/Sound/SE/shotArrow.mp3");
        m_seSlots.push_back({ h });
    }
    
}

void SoundManager::PlaySE(const std::string& name)
{
    auto it = m_sounds.find(name);
    if (it == m_sounds.end()) return;

    if(name == "shotArrow")
    {
        // 3重再生用スロットを探す
        for (auto& slot : m_seSlots)
        {
            if ( CheckSoundMem(slot.handle) == 0)
            {
              //二回目が鳴らない
                ChangeVolumeSoundMem(m_masterVolume2, slot.handle);
                PlaySoundMem(slot.handle, DX_PLAYTYPE_BACK);
                return;
            }
        }
        // 全てのスロットが再生中の場合、最初のスロットを強制的に再生
        auto& slot = m_seSlots[0];
        StopSoundMem(slot.handle);
        ChangeVolumeSoundMem(m_masterVolume2, slot.handle);
        PlaySoundMem(slot.handle, DX_PLAYTYPE_BACK);
        return;
	}
    else
    {
        ChangeVolumeSoundMem(m_seVolumes[name], it->second);
        PlaySoundMem(it->second, DX_PLAYTYPE_BACK);
    }


 
}

void SoundManager::PlayBgm(const std::string& name)
{
    auto it = m_sounds.find(name);
    if (it == m_sounds.end()) return;

    if (m_currentBgm != -1)
    {
        StopSoundMem(m_currentBgm);
    }

    m_currentBgm = it->second;//ハンドルの番号
    ChangeVolumeSoundMem(m_soundVolumes[name], m_currentBgm);
    PlaySoundMem(m_currentBgm, DX_PLAYTYPE_LOOP);
}

void SoundManager::PlaySELoop(const std::string& name)
{
	auto it = m_sounds.find(name);
	if (it == m_sounds.end()) return;
	//すでに再生中なら、再生しない   
	if (CheckSoundMem(it->second) == 1)return;
	ChangeVolumeSoundMem(m_seVolumes[name], it->second);
	PlaySoundMem(it->second, DX_PLAYTYPE_LOOP);
}

void SoundManager::StopBgm()
{
    if (m_currentBgm != -1)
    {
        StopSoundMem(m_currentBgm);
        m_currentBgm = -1;
    }
}

void SoundManager::StopSE(const std::string& name)
{
    auto it = m_sounds.find(name);
    if (it == m_sounds.end()) return;
    StopSoundMem(it->second);
}

void SoundManager::SetBgmVolume(int volume)
{
    m_masterVolume = volume;
    ChangeVolumeSoundMem(m_masterVolume, m_currentBgm);
}

void SoundManager::SetSEVolume(int volume)
{
    m_masterVolume2 = volume;
    //SEを流す
    PlaySE("cursor");
}

