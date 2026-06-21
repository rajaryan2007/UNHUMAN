#pragma once
#include "uhepch.h"
#include <box2d/types.h>
#include "SceneCamera.h"
#include "UHE/Animation/Animation2D/SpriteAnimation.h"
#include "UHE/Core/Core.h"
#include "UHE/Core/Timestep.h"
#include "UHE/Core/UIID.h"
#include "UHE/Renderer/Texture.h"
#include "UHE/Renderer3D/LoadModel.h"
#include "UHE/Renderer3D/Animator.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace UHE
{
struct UHE_API TransformComponent
{
    glm::vec3 Translation{0.0f};
    glm::vec3 Rotation{0.0f}; // assumed in degrees
    glm::vec3 Scale{1.0f};

    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;

    TransformComponent(const glm::vec3& translation) : Translation(translation) {}

    glm::mat4 GetTransform() const
    {
        glm::mat4 rotation = glm::mat4(glm::quat(Rotation));

        return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
    }
};

struct UHE_API IDComponent
{
    u64 ID = 0;
    IDComponent() = default;
    IDComponent(const IDComponent&) = default;
};

struct UHE_API SpriteRendererComponent
{
    glm::vec4 Color{1.0f};
    std::string TexturePath;
    Ref<Texture2D> Texture;
    f32 TilingFactor = 1.0f;

    bool UseSubTexture = false;
    glm::vec2 SubTextureCoords = {0.0f, 0.0f};
    glm::vec2 SubTextureCellSize = {16.0f, 16.0f};
    glm::vec2 SubTextureSpriteSize = {1.0f, 1.0f};

    SpriteRendererComponent() = default;
    SpriteRendererComponent(const SpriteRendererComponent&) = default;
    SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
};

struct UHE_API TagComponent
{
    std::string Tag;

    TagComponent() = default;
    TagComponent(const TagComponent&) = default;
    TagComponent(const std::string& tag) : Tag(tag) {}
};

struct UHE_API CameraComponent
{
    SceneCamera Camera;
    bool Primary = true;
    bool FixedAspectRatio = false;

    CameraComponent() = default;
    CameraComponent(const CameraComponent&) = default;
};

struct UHE_API SpriteAnimationComponent
{
    std::string SpriteSheetPath;
    SpriteAnimation Animation;
    glm::vec4 Color = {1.0f, 1.0f, 1.0f, 1.0f};

    SpriteAnimationComponent() = default;
    SpriteAnimationComponent(const SpriteAnimationComponent&) = default;
};

class ScriptableEntity;

struct UHE_API NativeScriptComponent
{
    ScriptableEntity* Instance = nullptr;

    ScriptableEntity* (*InstantiateScript)();
    void (*DestroyScript)(NativeScriptComponent*);

    template <typename T> void Bind()
    {
        InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
        DestroyScript = [](NativeScriptComponent* nsc)
        {
            delete nsc->Instance;
            nsc->Instance = nullptr;
        };
    }
};

struct UHE_API RigidBody2DComponent
{
    enum class BodyType
    {
        Static = 0,
        Kinematic,
        Dynamic
    };
    BodyType Type = BodyType::Static;
    bool FixedRotation = false;

    b2BodyId RuntimeBody = b2_nullBodyId;

    RigidBody2DComponent() = default;
    RigidBody2DComponent(const RigidBody2DComponent&) = default;
};

struct UHE_API Model3DComponent
{
    std::string ModelPath;
    Ref<RD3d::Model> ModelData = CreateRef<RD3d::Model>();
    bool IsLoaded = false;

    Model3DComponent() = default;
    Model3DComponent(const Model3DComponent&) = default;
};

struct UHE_API AnimatorComponent
{
    Ref<RD3d::Animator> Animator;
    bool IsPlaying = false;
    float PlaybackSpeed = 1.0f;
    std::string CurrentAnimationName = "";

    AnimatorComponent() = default;
    AnimatorComponent(const AnimatorComponent&) = default;
};

struct UHE_API DirectionalLightComponent
{
    glm::vec3 Color{1.0f, 1.0f, 1.0f};
    float Intensity = 1.0f;

    DirectionalLightComponent() = default;
    DirectionalLightComponent(const DirectionalLightComponent&) = default;
};

struct UHE_API PointLightComponent
{
    glm::vec3 Color{1.0f, 1.0f, 1.0f};
    float Intensity = 1.0f;
    float Radius = 10.0f;

    PointLightComponent() = default;
    PointLightComponent(const PointLightComponent&) = default;
};

struct UHE_API BoxColliderComponent
{
    glm::vec2 Offset{0.0f, 0.0f};
    glm::vec2 Size{0.5f, 0.5f};

    float Density = 1.0f;
    float Friction = 0.5f;
    float Restitution = 0.0f;
    float RestitutionThreshold = 0.5f;

    b2ShapeId RuntimeShape = b2_nullShapeId;

    BoxColliderComponent() = default;
    BoxColliderComponent(const BoxColliderComponent&) = default;
};

// --- 3D Physics Components ---

struct UHE_API RigidBody3DComponent
{
    enum class BodyType
    {
        Static = 0,
        Kinematic,
        Dynamic
    };
    BodyType Type = BodyType::Static;
    
    float Mass = 1.0f;
    float LinearDamping = 0.05f;
    float AngularDamping = 0.05f;
    bool AllowedDOFs[6] = {true, true, true, true, true, true}; // Translation X,Y,Z Rotation X,Y,Z
    bool IsSensor = false;

    // We store the JPH::BodyID as a uint32_t to avoid including Jolt headers everywhere
    uint32_t RuntimeBodyID = 0xFFFFFFFF; // JPH::BodyID::cInvalidBodyID

    RigidBody3DComponent() = default;
    RigidBody3DComponent(const RigidBody3DComponent&) = default;
};

struct UHE_API BoxCollider3DComponent
{
    glm::vec3 Offset{0.0f, 0.0f, 0.0f};
    glm::vec3 HalfExtent{0.5f, 0.5f, 0.5f};

    float Friction = 0.2f;
    float Restitution = 0.0f;

    BoxCollider3DComponent() = default;
    BoxCollider3DComponent(const BoxCollider3DComponent&) = default;
};

struct UHE_API SphereCollider3DComponent
{
    glm::vec3 Offset{0.0f, 0.0f, 0.0f};
    float Radius = 0.5f;

    float Friction = 0.2f;
    float Restitution = 0.0f;

    SphereCollider3DComponent() = default;
    SphereCollider3DComponent(const SphereCollider3DComponent&) = default;
};

struct UHE_API CapsuleCollider3DComponent
{
    glm::vec3 Offset{0.0f, 0.0f, 0.0f};
    float Radius = 0.5f;
    float HalfHeight = 1.0f; // Total height is 2 * (Radius + HalfHeight)

    float Friction = 0.2f;
    float Restitution = 0.0f;

    CapsuleCollider3DComponent() = default;
    CapsuleCollider3DComponent(const CapsuleCollider3DComponent&) = default;
};

} // namespace UHE
