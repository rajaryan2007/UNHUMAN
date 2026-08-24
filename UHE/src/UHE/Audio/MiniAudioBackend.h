#pragma once
#include <string>
#include <glm/glm.hpp>

namespace UHE::Audio {

class MiniAudioBackend
{
public:
    static void Init();
    static void Shutdown();
    static void Update();

    static void PlaySound2D(const std::string& filepath, float volume, bool loop);
    static void PlaySound3D(const std::string& filepath, const glm::vec3& position, float volume, bool loop);
    
    static void SetListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
};

} // namespace UHE::Audio
