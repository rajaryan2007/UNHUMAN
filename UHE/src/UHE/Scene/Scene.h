#pragma once

#include <box2d/types.h>
#include <example/stb_image.h>
#include "UHE/Core/Timestep.h"
#include "UHE/Renderer/EditorCamera.h"
#include "UHE/Scene/Components.h"
#include "entt.hpp"
class b2World;

namespace UHE
{

class Entity;
struct TransformComponent;
struct CameraComponent;
struct TagComponent;
struct SpriteRendererComponent;
struct SpriteAnimationComponent;
struct NativeScriptComponent;
struct RigidBody2DComponent;
struct BoxColliderComponent;
struct IDComponent;
struct Model3DComponent;

class UHE_API Scene
{
public:
    Scene();
    ~Scene();

    Entity CreateEntity(const std::string& name = std::string());

    void DestroyEntity(Entity entity);

    void OnViewportResize(u32 width, u32 height);

    void OnUpdateEditor(Timestep ts, EditorCamera& camera);
    void OnUpdateRuntime(Timestep ts);

    void OnRuntimeStart();
    void OnRuntimeStop();

    Entity GetPrimaryCameraEntity();

    static Ref<Scene> Copy(Ref<Scene> other);
    entt::registry& GetRegistry() { return m_registry; }

    static bool& GetShowLightIcons() { static bool s_ShowLightIcons = true; return s_ShowLightIcons; }

public:
    template <typename T> void OnComponentAdded(Entity entity, T& components);

private:
    void RenderSprites(Timestep ts);
    void RenderModels(Timestep ts);
    void RenderLightIcons(EditorCamera& camera);

    entt::registry m_registry;
    u32 m_ViewportWidth = 0, m_ViewportHeight = 0;

    b2WorldId m_PhysicsWorldId = b2_nullWorldId;
    
    Ref<Texture2D> m_DirLightIcon;
    Ref<Texture2D> m_PointLightIcon;

    friend class SceneSerializer;
    friend class Entity;
    friend class SceneHierarchyPanel;
};

template <> UHE_API void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& components);
template <> UHE_API void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& components);
template <> UHE_API void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& components);
template <> UHE_API void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& components);
template <> UHE_API void Scene::OnComponentAdded<SpriteAnimationComponent>(Entity entity, SpriteAnimationComponent& components);
template <> UHE_API void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& components);
template <> UHE_API void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& components);
template <> UHE_API void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& components);
template <> UHE_API void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& components);
template <> UHE_API void Scene::OnComponentAdded<Model3DComponent>(Entity entity, Model3DComponent& components);
template <> UHE_API void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& components);
template <> UHE_API void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& components);
template <> UHE_API void Scene::OnComponentAdded<AnimatorComponent>(Entity entity, AnimatorComponent& components);

} // namespace UHE
