#include "uhepch.h"
#include "LightSystem.h"
#include "UHE/Scene/Components.h"
#include <glm/gtx/quaternion.hpp>

namespace UHE::RD3d
{

std::vector<LightData> LightSystem::ExtractLights(entt::registry& registry)
{
    std::vector<LightData> lights;
    
    // Default fallback if no lights exist
    if (registry.view<DirectionalLightComponent>().empty() && registry.view<PointLightComponent>().empty())
    {
        LightData fallback;
        fallback.Type_Radius_Pad = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // Type = 0 (Directional)
        fallback.PositionOrDirection = glm::vec4(0.5f, -1.0f, 0.3f, 0.0f);
        fallback.ColorIntensity = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        lights.push_back(fallback);
        return lights;
    }

    auto dirLightView = registry.view<TransformComponent, DirectionalLightComponent>();
    for (auto entity : dirLightView)
    {
        auto [transform, light] = dirLightView.get<TransformComponent, DirectionalLightComponent>(entity);
        
        glm::quat q = glm::quat(transform.Rotation);
        glm::vec3 direction = glm::normalize(q * glm::vec3(0.0f, 0.0f, -1.0f));
        
        LightData data;
        data.Type_Radius_Pad = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // Type = 0
        data.PositionOrDirection = glm::vec4(direction, 0.0f);
        data.ColorIntensity = glm::vec4(light.Color, light.Intensity);
        lights.push_back(data);
    }
    
    auto pointLightView = registry.view<TransformComponent, PointLightComponent>();
    for (auto entity : pointLightView)
    {
        auto [transform, light] = pointLightView.get<TransformComponent, PointLightComponent>(entity);
        
        LightData data;
        data.Type_Radius_Pad = glm::vec4(1.0f, light.Radius, 0.0f, 0.0f); // Type = 1
        data.PositionOrDirection = glm::vec4(transform.Translation, 1.0f);
        data.ColorIntensity = glm::vec4(light.Color, light.Intensity);
        lights.push_back(data);
    }
    
    return lights;
}

} // namespace UHE::RD3d
