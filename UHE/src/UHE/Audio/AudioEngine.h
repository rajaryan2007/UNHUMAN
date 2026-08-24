#pragma once
#include "UHE/Core/Core.h"
#include <string>
#include <glm/glm.hpp>

namespace UHE::Audio {

class UHE_API AudioEngine
{
public:
    static void Init();
    static void Shutdown();
    static void Update();

    // 2D Audio
    static void PlaySound2D(const std::string& filepath, float volume = 1.0f, bool loop = false);
    
    // 3D Spatial Audio
    static void PlaySound3D(const std::string& filepath, const glm::vec3& position, float volume = 1.0f, bool loop = false);
    
    // Set the listener (camera) position and direction for 3D audio calculations
    static void SetListenerPosition(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
};

} // namespace UHE::Audio
