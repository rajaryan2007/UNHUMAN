#include "uhepch.h"
#include "AudioEngine.h"
#include "MiniAudioBackend.h"

namespace UHE::Audio {

void AudioEngine::Init()
{
    MiniAudioBackend::Init();
}

void AudioEngine::Shutdown()
{
    MiniAudioBackend::Shutdown();
}

void AudioEngine::Update()
{
    MiniAudioBackend::Update();
}

void AudioEngine::PlaySound2D(const std::string& filepath, float volume, bool loop)
{
    MiniAudioBackend::PlaySound2D(filepath, volume, loop);
}

void AudioEngine::PlaySound3D(const std::string& filepath, const glm::vec3& position, float volume, bool loop)
{
    MiniAudioBackend::PlaySound3D(filepath, position, volume, loop);
}

void AudioEngine::SetListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
{
    MiniAudioBackend::SetListenerPosition(position, forward, up);
}

} // namespace UHE::Audio
