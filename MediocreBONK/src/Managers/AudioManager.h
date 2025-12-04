#pragma once
#include "../Core/ResourceManager.h"
#include "../Utils/Logger.h"
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

namespace MediocreBONK::Managers
{
    enum class SoundType
    {
        PlayerShoot,
        PlayerHit,
        PlayerDash,
        EnemyHit,
        EnemyDeath,
        XPCollect,
        PowerUpCollect,
        LevelUp,
        Explosion
    };

    enum class MusicType
    {
        Menu,
        GameplayCalm,
        GameplayIntense,
        BossFight
    };

    /*
     * DESIGN PATTERN: SINGLETON PATTERN + OBJECT POOLING
     *
     * AudioManager combines two patterns:
     *
     * 1. SINGLETON PATTERN:
     *    - Single instance manages all audio in the game
     *    - Centralized control over volume, enabled/disabled state
     *
     * 2. OBJECT POOLING PATTERN:
     *    - Pre-allocates a pool of sf::Sound objects
     *    - Reuses sound objects instead of creating/destroying them
     *    - Prevents memory allocation during gameplay (performance)
     *
     * Object Pooling Benefits:
     * - No memory allocation during sound playback (zero-cost audio)
     * - Predictable memory usage (fixed pool size)
     * - Cache-friendly (sounds stored contiguously in vector)
     *
     * Trade-offs:
     * - Limited simultaneous sounds (pool size = 32)
     * - Oldest sound gets interrupted if pool is full
     */
    class AudioManager
    {
    public:
        // SINGLETON PATTERN: Global access to audio system
        static AudioManager& getInstance()
        {
            static AudioManager instance;
            return instance;
        }

        // SINGLETON PATTERN: Prevent copying
        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;

        // OBJECT POOLING: Initialize the pool of reusable sound objects
        void initialize()
        {
            // Pre-allocate 32 sf::Sound objects (object pool)
            soundPool.resize(32); // 32 concurrent sounds max
            for (auto& sound : soundPool)
            {
                sound = std::make_unique<sf::Sound>(dummyBuffer); // Create sound object
            }

            Utils::Logger::info("AudioManager initialized with 32 sound slots");
        }

        // Load sound buffer
        void loadSound(SoundType type, const std::string& filepath)
        {
            try
            {
                const auto& buffer = Core::ResourceManager::getInstance().getSoundBuffer(filepath);
                soundBuffers[type] = &buffer;
            }
            catch (const std::exception& e)
            {
                Utils::Logger::warning("Failed to load sound: " + filepath);
            }
        }

        // Load music
        void loadMusic(MusicType type, const std::string& filepath)
        {
            auto music = std::make_unique<sf::Music>();
            if (!music->openFromFile(filepath))
            {
                Utils::Logger::warning("Failed to load music: " + filepath);
                return;
            }

            musicTracks[type] = std::move(music);
        }

        // OBJECT POOLING: Reuse sound objects from the pool
        void playSound(SoundType type, float volume = 100.f, float pitch = 1.f)
        {
            if (!soundsEnabled)
                return;

            auto it = soundBuffers.find(type);
            if (it == soundBuffers.end())
                return;

            // OBJECT POOLING: Find an available (not playing) sound in the pool
            sf::Sound* availableSound = nullptr;
            for (auto& sound : soundPool)
            {
                // Check if this sound slot is available (not currently playing)
                if (sound->getStatus() != sf::Sound::Status::Playing)
                {
                    availableSound = sound.get();
                    break;
                }
            }

            if (!availableSound)
            {
                // Pool exhausted: all 32 sounds are playing
                // Strategy: Interrupt oldest sound (first in pool)
                availableSound = soundPool[0].get();
            }

            // Reuse the sound object (no allocation!)
            availableSound->setBuffer(*it->second);
            availableSound->setVolume(volume * masterVolume * soundVolume);
            availableSound->setPitch(pitch);
            availableSound->play();
        }

        // Play music
        void playMusic(MusicType type, bool loop = true)
        {
            if (!musicEnabled)
                return;

            stopMusic(); // Stop current music first

            auto it = musicTracks.find(type);
            if (it == musicTracks.end())
                return;

            currentMusic = it->second.get();
            currentMusic->setLooping(loop);
            currentMusic->setVolume(masterVolume * musicVolume);
            currentMusic->play();
        }

        void stopMusic()
        {
            if (currentMusic)
            {
                currentMusic->stop();
                currentMusic = nullptr;
            }
        }

        void pauseMusic()
        {
            if (currentMusic)
                currentMusic->pause();
        }

        void resumeMusic()
        {
            if (currentMusic)
                currentMusic->play();
        }

        // Volume controls (0-100)
        void setMasterVolume(float volume)
        {
            masterVolume = std::clamp(volume, 0.f, 100.f) / 100.f;
            if (currentMusic)
                currentMusic->setVolume(masterVolume * musicVolume * 100.f);
        }

        void setSoundVolume(float volume)
        {
            soundVolume = std::clamp(volume, 0.f, 100.f) / 100.f;
        }

        void setMusicVolume(float volume)
        {
            musicVolume = std::clamp(volume, 0.f, 100.f) / 100.f;
            if (currentMusic)
                currentMusic->setVolume(masterVolume * musicVolume * 100.f);
        }

        void setSoundsEnabled(bool enabled) { soundsEnabled = enabled; }
        void setMusicEnabled(bool enabled)
        {
            musicEnabled = enabled;
            if (!enabled)
                stopMusic();
        }

        bool isSoundsEnabled() const { return soundsEnabled; }
        bool isMusicEnabled() const { return musicEnabled; }

        float getMasterVolume() const { return masterVolume * 100.f; }
        float getSoundVolume() const { return soundVolume * 100.f; }
        float getMusicVolume() const { return musicVolume * 100.f; }

    private:
        // SINGLETON PATTERN: Private constructor
        AudioManager()
            : masterVolume(0.7f)
            , soundVolume(1.0f)
            , musicVolume(0.6f)
            , soundsEnabled(true)
            , musicEnabled(true)
            , currentMusic(nullptr)
        {}

        // Dummy buffer for SFML 3 compatibility (sf::Sound requires a buffer on construction)
        sf::SoundBuffer dummyBuffer;

        // OBJECT POOLING: Pre-allocated pool of reusable sound objects
        // Fixed size (32) means predictable memory usage and no runtime allocation
        std::vector<std::unique_ptr<sf::Sound>> soundPool;

        // Resource references: maps sound types to loaded buffers
        std::unordered_map<SoundType, const sf::SoundBuffer*> soundBuffers;
        std::unordered_map<MusicType, std::unique_ptr<sf::Music>> musicTracks;

        // Currently playing music track
        sf::Music* currentMusic;

        // Volume controls (0.0 to 1.0 range internally)
        float masterVolume;
        float soundVolume;
        float musicVolume;
        bool soundsEnabled;
        bool musicEnabled;
    };
}
