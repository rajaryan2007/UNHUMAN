#include "uhepch.h"
#include "MiniAudioBackend.h"
#include "UHE/Core/Log.h"

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_FLAC // Bypass dr_flac GCC -O0 SSE4.1 compiler bug
#define MA_NO_MP3
#include <miniaudio.h>

#include <vector>

namespace UHE::Audio {

static ma_engine s_AudioEngine;
static bool s_Initialized = false;

// We store pointers to active fire-and-forget sounds here and clean them up in Update()
static std::vector<ma_sound*> s_ActiveSounds;

void MiniAudioBackend::Init()
{
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.listenerCount = 1;

    ma_result result = ma_engine_init(&engineConfig, &s_AudioEngine);
    if (result != MA_SUCCESS)
    {
        UHE_ERROR("Failed to initialize MiniAudio engine. Error code: {}", (int)result);
        return;
    }

    s_Initialized = true;
    UHE_INFO("MiniAudio Backend Initialized Successfully");
}

void MiniAudioBackend::Shutdown()
{
    if (s_Initialized)
    {
        for (ma_sound* sound : s_ActiveSounds)
        {
            ma_sound_uninit(sound);
            delete sound;
        }
        s_ActiveSounds.clear();

        ma_engine_uninit(&s_AudioEngine);
        s_Initialized = false;
        UHE_INFO("MiniAudio Backend Shut Down");
    }
}

void MiniAudioBackend::Update()
{
    if (!s_Initialized) return;

    // Clean up sounds that have finished playing
    for (auto it = s_ActiveSounds.begin(); it != s_ActiveSounds.end(); )
    {
        ma_sound* sound = *it;
        if (!ma_sound_is_playing(sound))
        {
            ma_sound_uninit(sound);
            delete sound;
            it = s_ActiveSounds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void MiniAudioBackend::PlaySound2D(const std::string& filepath, float volume, bool loop)
{
    if (!s_Initialized) return;

    ma_sound* sound = new ma_sound;
    ma_result result = ma_sound_init_from_file(&s_AudioEngine, filepath.c_str(), 0, nullptr, nullptr, sound);

    if (result == MA_SUCCESS) {
        ma_sound_set_volume(sound, volume);
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
        ma_sound_start(sound);
        
        s_ActiveSounds.push_back(sound);
    }
    else {
        UHE_ERROR("Failed to load 2D sound: {}", filepath);
        delete sound;
    }
}

void MiniAudioBackend::PlaySound3D(const std::string& filepath, const glm::vec3& position, float volume, bool loop)
{
    if (!s_Initialized) return;

    ma_sound* sound = new ma_sound;
    
    // Initialize sound with spatialization enabled
    ma_result result = ma_sound_init_from_file(&s_AudioEngine, filepath.c_str(), 0, nullptr, nullptr, sound);

    if (result == MA_SUCCESS) {
        ma_sound_set_position(sound, position.x, position.y, position.z);
        ma_sound_set_volume(sound, volume);
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
        
        ma_sound_start(sound);
        
        s_ActiveSounds.push_back(sound);
    }
    else {
        UHE_ERROR("Failed to load 3D sound: {}", filepath);
        delete sound;
    }
}

void MiniAudioBackend::SetListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
    if (!s_Initialized) return;

    ma_engine_listener_set_position(&s_AudioEngine, 0, position.x, position.y, position.z);
    ma_engine_listener_set_direction(&s_AudioEngine, 0, forward.x, forward.y, forward.z);
    ma_engine_listener_set_world_up(&s_AudioEngine, 0, up.x, up.y, up.z);
}

} // namespace UHE::Audio
