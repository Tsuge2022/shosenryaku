#pragma once
#include <Siv3D.hpp> // For Audio, FilePath, String, HashTable

enum class SoundEffect {
    TurnEnd,    // tEndSound
    Explosion1, // boomSound
    Gunfire,    // gunSound
    Cannon,     // cannonSound
    BigExplosion, // bigBoomSound
    Victory,    // endSound (for game end)
    Purchase,   // buySound
    // Consider adding:
    // UnitSelect,
    // UnitMoveConfirmation,
    // UIClick,
    // Error, // For invalid actions
    // BuildingCaptured
};

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    void LoadSounds();
    void PlayBGM(const FilePath& bgmPath, bool loop = true, double volume = 1.0); // volume is relative to master BGM vol
    void StopBGM();
    void PauseBGM();
    void ResumeBGM();

    void PlaySFX(SoundEffect sfx, double volume = 1.0); // volume is relative to master SFX vol

    void SetMasterVolume(double volume);
    void SetBGMVolume(double volume);    // Sets the master BGM volume
    void SetSFXVolume(double volume);    // Sets the master SFX volume

private:
    Audio backgroundMusic;
    HashTable<SoundEffect, Audio> soundEffectPlayers; // Renamed for clarity

    double masterVolume = 1.0;
    double bgmMasterVolume = 1.0;
    double sfxMasterVolume = 1.0;

    // Store default paths
    const FilePath defaultBGMPath = U"App/bgm&se/BGM.mp3"; // Corrected path based on ls output
    HashTable<SoundEffect, FilePath> sfxPaths = {
        {SoundEffect::TurnEnd, U"App/bgm&se/決定ボタン.mp3"},
        {SoundEffect::Explosion1, U"App/bgm&se/爆発1.mp3"},
        {SoundEffect::Gunfire, U"App/bgm&se/重機関銃を乱射1.mp3"},
        {SoundEffect::Cannon, U"App/bgm&se/大砲1.mp3"},
        {SoundEffect::BigExplosion, U"App/bgm&se/爆発2.mp3"},
        {SoundEffect::Victory, U"App/bgm&se/歓声と拍手.mp3"},
        {SoundEffect::Purchase, U"App/bgm&se/レジスターで精算.mp3"}
        // Add other default paths here
    };
};
