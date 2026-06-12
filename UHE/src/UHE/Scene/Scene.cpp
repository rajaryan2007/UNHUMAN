#include "uhepch.h"
#include "Scene.h"
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include "Components.h"
#include "Entity.h"
#include "ScriptableEntity.h"
#include "UHE/Core/UIID.h"
#include "UHE/Renderer/Renderer2D.h"
#include "UHE/Renderer2D/SubTexture2D.h"

namespace UHE
{

Scene::Scene()
{
    // entt::entity entity = m_registry.create();
    //
    // m_registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));
    //
    // if (m_registry.all_of<TransformComponent>(entity))
    // {
    //     auto& transform = m_registry.get<TransformComponent>(entity);
    // }
    //
    // auto view = m_registry.view<TransformComponent>();
    //
    // for (auto entity : view)
    // {
    //     auto& transform = view.get<TransformComponent>(entity);
    // }
    //
    // auto group =
    // m_registry.group<TransformComponent>(entt::get<MeshComponent>);
    //
    // for (auto entity : group)
    // {
    //     auto& [transform, mesh] = group.get<TransformComponent,
    //     MeshComponent>(entity);
    // }
}

Scene::~Scene() = default;

Ref<Scene> Scene::Copy(Ref<Scene> other)
{
    Ref<Scene> newScene = CreateRef<Scene>();
    newScene->m_ViewportWidth = other->m_ViewportWidth;
    newScene->m_ViewportHeight = other->m_ViewportHeight;

    auto& srcRegistry = other->m_registry;
    auto& dstRegistry = newScene->m_registry;

    auto view = srcRegistry.view<TagComponent>();
    for (auto srcEntity : view)
    {
        const auto& tag = srcRegistry.get<TagComponent>(srcEntity).Tag;
        Entity newEntity = newScene->CreateEntity(tag);

        // Copy TransformComponent
        if (srcRegistry.all_of<TransformComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<TransformComponent>(srcEntity);
            auto& dst = newEntity.GetComponent<TransformComponent>();
            dst.Translation = src.Translation;
            dst.Rotation = src.Rotation;
            dst.Scale = src.Scale;
        }

        // Copy CameraComponent
        if (srcRegistry.all_of<CameraComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<CameraComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<CameraComponent>();
            dst.Camera = src.Camera;
            dst.Primary = src.Primary;
            dst.FixedAspectRatio = src.FixedAspectRatio;
        }

        // Copy SpriteRendererComponent
        if (srcRegistry.all_of<SpriteRendererComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<SpriteRendererComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<SpriteRendererComponent>();
            dst.Color = src.Color;
            dst.TexturePath = src.TexturePath;
            dst.Texture = src.Texture;
            dst.TilingFactor = src.TilingFactor;
            dst.UseSubTexture = src.UseSubTexture;
            dst.SubTextureCoords = src.SubTextureCoords;
            dst.SubTextureCellSize = src.SubTextureCellSize;
            dst.SubTextureSpriteSize = src.SubTextureSpriteSize;
        }

        // Copy SpriteAnimationComponent
        if (srcRegistry.all_of<SpriteAnimationComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<SpriteAnimationComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<SpriteAnimationComponent>();
            dst.SpriteSheetPath = src.SpriteSheetPath;
            dst.Animation = src.Animation;
            dst.Color = src.Color;
        }

        // Copy NativeScriptComponent
        if (srcRegistry.all_of<NativeScriptComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<NativeScriptComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<NativeScriptComponent>();
            dst.InstantiateScript = src.InstantiateScript;
            dst.DestroyScript = src.DestroyScript;
        }

        // Copy RigidBody2DComponent
        if (srcRegistry.all_of<RigidBody2DComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<RigidBody2DComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<RigidBody2DComponent>();
            dst.Type = src.Type;
            dst.FixedRotation = src.FixedRotation;
        }

        // Copy BoxColliderComponent
        if (srcRegistry.all_of<BoxColliderComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<BoxColliderComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<BoxColliderComponent>();
            dst.Offset = src.Offset;
            dst.Size = src.Size;
            dst.Density = src.Density;
            dst.Friction = src.Friction;
            dst.Restitution = src.Restitution;
            dst.RestitutionThreshold = src.RestitutionThreshold;
        }
    }

    return newScene;
}

UHE::Entity Scene::CreateEntity(const std::string& name /*= std::string()*/)
{
    Entity entity = {m_registry.create(), this};
    entity.AddComponent<IDComponent>();
    entity.AddComponent<TransformComponent>();
    auto& tag = entity.AddComponent<TagComponent>();
    tag.Tag = name.empty() ? "Entity" : name;

    return entity;
}

void Scene::DestroyEntity(Entity entity)
{
    m_registry.destroy(entity);
}

void Scene::OnViewportResize(u32 width, u32 height)
{
    m_ViewportWidth = width;
    m_ViewportHeight = height;

    auto view = m_registry.view<CameraComponent>();
    for (auto entity : view)
    {
        auto& cameraComponent = view.get<CameraComponent>(entity);
        if (!cameraComponent.FixedAspectRatio)
        {
            cameraComponent.Camera.SetViewportSize(width, height);
        }
    }
}

void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
{
    m_registry.view<NativeScriptComponent>().each(
        [=](auto entity, auto& nsc)
        {
            if (!nsc.Instance)
            {
                nsc.Instance = nsc.InstantiateScript();
                nsc.Instance->m_Entity = Entity{entity, this};
                nsc.Instance->OnCreate();
            }
            nsc.Instance->OnUpdate(ts);
        });

    Renderer2D::BeginScene(camera);
    RenderSprites(ts);
    Renderer2D::EndScene();
}

void Scene::RenderSprites(Timestep ts)
{
    auto animView = m_registry.view<SpriteAnimationComponent>();
    for (auto entity : animView)
    {
        auto& comp = animView.get<SpriteAnimationComponent>(entity);
        comp.Animation.Tick(ts);
    }

    auto spriteView = m_registry.view<TransformComponent, SpriteRendererComponent>();
    for (auto entityID : spriteView)
    {
        auto [transform, sprite] = spriteView.get<TransformComponent, SpriteRendererComponent>(entityID);

        Renderer2D::DrawSprite(transform.GetTransform(), sprite, (i32)entityID);
    }

    auto animOnlyView = m_registry.view<TransformComponent, SpriteAnimationComponent>();
    for (auto entityID : animOnlyView)
    {
        if (m_registry.all_of<SpriteRendererComponent>(entityID))
            continue;

        auto [transform, comp] = animOnlyView.get<TransformComponent, SpriteAnimationComponent>(entityID);

        Ref<SubTexture2D> overrideSubTex = comp.Animation.GetCurrentFrame();
        if (overrideSubTex)
        {
            Renderer2D::DrawQuad(transform.GetTransform(), overrideSubTex, 1.0f, comp.Color);
        }
    }
}

void Scene::OnUpdateRuntime(Timestep ts)
{

    {

        m_registry.view<NativeScriptComponent>().each(
            [=](auto entity, auto& nsc)
            {
                if (!nsc.Instance)
                {
                    nsc.Instance = nsc.InstantiateScript();
                    nsc.Instance->m_Entity = Entity{entity, this};
                    nsc.Instance->OnCreate();
                }
                nsc.Instance->OnUpdate(ts);
            });
    }

    // Physics
    {
        const int subStepCount = 4;
        b2World_Step(m_PhysicsWorldId, ts, subStepCount);

        // Retrieve transforms from Box2D
        auto view = m_registry.view<TransformComponent, RigidBody2DComponent>();
        for (auto entity : view)
        {
            auto [transform, rb2d] = view.get<TransformComponent, RigidBody2DComponent>(entity);

            if (!b2Body_IsValid(rb2d.RuntimeBody))
                continue;

            b2Vec2 position = b2Body_GetPosition(rb2d.RuntimeBody);
            transform.Translation.x = position.x;
            transform.Translation.y = position.y;

            b2Rot rotation = b2Body_GetRotation(rb2d.RuntimeBody);
            transform.Rotation.z = b2Rot_GetAngle(rotation);
        }
    }

    Camera* mainCamera = nullptr;
    glm::mat4 cameraTransform;

    auto view = m_registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view)
    {
        auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

        if (camera.Primary)
        {
            mainCamera = &camera.Camera;
            cameraTransform = transform.GetTransform();
            break;
        }
    }

    if (mainCamera)
    {
        Renderer2D::BeginScene(*mainCamera, cameraTransform);
        RenderSprites(ts);
        Renderer2D::EndScene();
    }
}

void Scene::OnRuntimeStart()
{
    // 1. Define and Create the World
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -9.8f};
    m_PhysicsWorldId = b2CreateWorld(&worldDef);

    auto view = m_registry.view<TransformComponent, RigidBody2DComponent, BoxColliderComponent>();

    for (auto entity : view)
    {
        auto [transform, rb2d, bc2d] = view.get<TransformComponent, RigidBody2DComponent, BoxColliderComponent>(entity);

        // --- Body Configuration ---
        b2BodyDef bodyDef = b2DefaultBodyDef();

        switch (rb2d.Type)
        {
            case RigidBody2DComponent::BodyType::Static:
                bodyDef.type = b2_staticBody;
                break;
            case RigidBody2DComponent::BodyType::Dynamic:
                bodyDef.type = b2_dynamicBody;
                break;
            case RigidBody2DComponent::BodyType::Kinematic:
                bodyDef.type = b2_kinematicBody;
                break;
        }

        bodyDef.position = {transform.Translation.x, transform.Translation.y};
        // Ensure Rotation.z is in Radians!
        bodyDef.rotation = b2MakeRot(transform.Rotation.z);
        bodyDef.motionLocks.angularZ = rb2d.FixedRotation;

        // Create the body
        b2BodyId bodyId = b2CreateBody(m_PhysicsWorldId, &bodyDef);

        // FIX: Store the whole struct, not just the index
        rb2d.RuntimeBody = bodyId;

        // --- Shape/Fixture Configuration ---
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = bc2d.Density;
        // Properties moved into the material sub-struct
        shapeDef.material.friction = bc2d.Friction;
        shapeDef.material.restitution = bc2d.Restitution;

        // Box2D v3 uses half-extents (width/2, height/2), scaled by transform
        b2Polygon box = b2MakeOffsetBox(bc2d.Size.x * transform.Scale.x * 0.5f, bc2d.Size.y * transform.Scale.y * 0.5f,
                                        {bc2d.Offset.x, bc2d.Offset.y},
                                        b2MakeRot(0.0f) // The rotation of the box itself relative to body
        );

        b2CreatePolygonShape(bodyId, &shapeDef, &box);
    }
}

void Scene::OnRuntimeStop()
{
    if (b2World_IsValid(m_PhysicsWorldId))
    {
        b2DestroyWorld(m_PhysicsWorldId);
        m_PhysicsWorldId = b2_nullWorldId;
    }
}

Entity Scene::GetPrimaryCameraEntity()
{
    auto view = m_registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view)
    {
        const auto& camera = view.get<CameraComponent>(entity);
        if (camera.Primary)
            return Entity{entity, this};
    }
    return {};
}

template <typename T> void Scene::OnComponentAdded(Entity entity, T& components)
{
    static_assert(sizeof(T) == 0, "Only specialized components can be added!");
}

template <> void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& components) {}
template <> void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& components)
{
    components.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
}

template <> void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& components) {}
template <> void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& components) {}
template <> void Scene::OnComponentAdded<SpriteAnimationComponent>(Entity entity, SpriteAnimationComponent& components)
{
}
template <> void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& components) {}
template <> void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& components) {}
template <> void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& components) {}
template <> void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& components) {};
template <> void Scene::OnComponentAdded<Model3DComponent>(Entity entity, Model3DComponent& components) {};

template void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& components);
template void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& components);
template void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& components);
template void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& components);
template void Scene::OnComponentAdded<SpriteAnimationComponent>(Entity entity, SpriteAnimationComponent& components);
template void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& components);
template void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& components);
template void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& components);
template void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& components);
template void Scene::OnComponentAdded<Model3DComponent>(Entity entity, Model3DComponent& components);
} // namespace UHE
