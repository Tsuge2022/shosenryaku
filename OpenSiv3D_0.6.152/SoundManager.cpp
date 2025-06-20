#include "SoundManager.h"

SoundManager::SoundManager() {
    LoadSounds(); // Load sounds upon construction
}

SoundManager::~SoundManager() {
    // Audio objects are managed by Siv3D and will release resources when destroyed.
    // Stop any playing sounds to be clean, though Siv3D might handle this.
    if (backgroundMusic.isPlaying()) {
        backgroundMusic.stop();
    }
    soundEffectPlayers.clear();
}

void SoundManager::LoadSounds() {
    // BGM is loaded on PlayBGM to allow flexibility in choosing the track.
    // Pre-load SFX
    soundEffectPlayers.clear(); // Clear any existing
    for (const auto& pair : sfxPaths) {
        soundEffectPlayers.emplace(pair.first, Audio{pair.second});
        // Apply initial volume to the loaded SFX Audio object
        if (soundEffectPlayers.contains(pair.first)) {
            soundEffectPlayers[pair.first].setVolume(masterVolume * sfxMasterVolume);
        }
    }
}

void SoundManager::PlayBGM(const FilePath& bgmPath, bool loop, double volume) {
    if (backgroundMusic.isPlaying()) {
        backgroundMusic.stop();
    }
    backgroundMusic = Audio{bgmPath, loop ? Loop::Yes : Loop::No};
    backgroundMusic.setVolume(masterVolume * bgmMasterVolume * volume);
    backgroundMusic.play();
}

void SoundManager::StopBGM() {
    backgroundMusic.stop();
}

void SoundManager::PauseBGM() {
    backgroundMusic.pause();
}

void SoundManager::ResumeBGM() {
    if (backgroundMusic.isPaused()) {
        backgroundMusic.play(); // Siv3D's play() resumes if paused
    } else if (!backgroundMusic.isPlaying() && backgroundMusic.isLoaded()) {
        // If stopped but loaded, play from beginning or a stored position
        backgroundMusic.play();
    }
}

void SoundManager::PlaySFX(SoundEffect sfx, double volume) {
    if (soundEffectPlayers.contains(sfx)) {
        // Create a temporary Audio object for one-shot play to allow overlapping sounds of the same type.
        // However, the original code used member Audio objects and called playOneShot.
        // If we want to use the pre-loaded Audio objects to manage their volume globally:
        soundEffectPlayers[sfx].setVolume(masterVolume * sfxMasterVolume * volume);
        soundEffectPlayers[sfx].playOneShot();
        // If truly independent one-shots are needed without affecting the stored Audio object's volume permanently:
        // Audio sfxPlayer = soundEffectPlayers[sfx]; // This copies the Audio object state
        // sfxPlayer.setVolume(masterVolume * sfxMasterVolume * volume);
        // sfxPlayer.playOneShot();
        // For simplicity and consistency with original, setting volume on the stored player then playOneShot.
    }
}

void SoundManager::SetMasterVolume(double volume) {
    masterVolume = Clamp(volume, 0.0, 1.0);
    SetBGMVolume(bgmMasterVolume); // Re-apply BGM master volume with new overall master
    SetSFXVolume(sfxMasterVolume); // Re-apply SFX master volume
}

void SoundManager::SetBGMVolume(double volume) {
    bgmMasterVolume = Clamp(volume, 0.0, 1.0);
    if (backgroundMusic.isLoaded()) { // Check if BGM has been loaded/played at least once
        backgroundMusic.setVolume(masterVolume * bgmMasterVolume);
    }
}

void SoundManager::SetSFXVolume(double volume) {
    sfxMasterVolume = Clamp(volume, 0.0, 1.0);
    // Update volume for all pre-loaded SFX players
    for (auto& pair : soundEffectPlayers) {
        pair.second.setVolume(masterVolume * sfxMasterVolume);
    }
}
