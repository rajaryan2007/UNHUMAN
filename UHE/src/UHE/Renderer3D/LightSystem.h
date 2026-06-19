#pragma once
#include "UHE/Core/Core.h"
#include <glm/glm.hpp>
#include "entt.hpp"
#include <vector>

namespace UHE::RD3d
{

struct LightData {
    glm::vec4 Type_Radius_Pad; // x = type (0: Dir, 1: Point), y = radius, z, w = pad
    glm::vec4 PositionOrDirection; // w = pad
    glm::vec4 ColorIntensity;
};

class UHE_API LightSystem
{
public:
    static std::vector<LightData> ExtractLights(entt::registry& registry);
};

} // namespace UHE::RD3d
