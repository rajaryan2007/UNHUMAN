#include "uhepch.h"
#include "Scene.h"
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include "Components.h"
#include "Entity.h"
#include "ScriptableEntity.h"
#include "UHE/Core/UIID.h"
#include "UHE/Renderer/Renderer2D.h"
#include "UHE/Renderer3D/Renderer3D.h"
#include "UHE/Renderer2D/SubTexture2D.h"
#include "UHE/Renderer3D/LightSystem.h"
#include "UHE/AssestsManager/VfsSystem.h"

// Jolt Physics
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

namespace UHE
{

Scene::Scene()
{
    // Initialize all component pools to prevent EnTT out-of-bounds asserts across DLL boundaries
    m_registry.storage<IDComponent>();
    m_registry.storage<TagComponent>();
    m_registry.storage<TransformComponent>();
    m_registry.storage<CameraComponent>();
    m_registry.storage<SpriteRendererComponent>();
    m_registry.storage<SpriteAnimationComponent>();
    m_registry.storage<NativeScriptComponent>();
    m_registry.storage<RigidBody2DComponent>();
    m_registry.storage<BoxColliderComponent>();
    m_registry.storage<Model3DComponent>();
    m_registry.storage<RigidBody3DComponent>();
    m_registry.storage<BoxCollider3DComponent>();
    m_registry.storage<SphereCollider3DComponent>();
    m_registry.storage<CapsuleCollider3DComponent>();
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
        // Copy Model3DComponent
        if (srcRegistry.all_of<Model3DComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<Model3DComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<Model3DComponent>();
            dst.ModelPath = src.ModelPath;
            dst.IsLoaded = src.IsLoaded;
            dst.ModelData = src.ModelData;
        }

        // Copy AnimatorComponent
        if (srcRegistry.all_of<AnimatorComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<AnimatorComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<AnimatorComponent>();
            dst.CurrentAnimationName = src.CurrentAnimationName;
            dst.PlaybackSpeed = src.PlaybackSpeed;
            dst.IsPlaying = src.IsPlaying;
            // Note: Animator itself needs to be recreated since it depends on the ModelData instance, 
            // but we can just share it or let it re-initialize in OnUpdate. We'll copy the ref.
            dst.Animator = src.Animator;
        }

        // Copy DirectionalLightComponent
        if (srcRegistry.all_of<DirectionalLightComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<DirectionalLightComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<DirectionalLightComponent>();
            dst.Color = src.Color;
            dst.Intensity = src.Intensity;
        }

        // Copy PointLightComponent
        if (srcRegistry.all_of<PointLightComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<PointLightComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<PointLightComponent>();
            dst.Color = src.Color;
            dst.Intensity = src.Intensity;
            dst.Radius = src.Radius;
        }

        // Copy RigidBody3DComponent
        if (srcRegistry.all_of<RigidBody3DComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<RigidBody3DComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<RigidBody3DComponent>();
            dst.Type = src.Type;
            dst.Mass = src.Mass;
            dst.LinearDamping = src.LinearDamping;
            dst.AngularDamping = src.AngularDamping;
            for (int i = 0; i < 6; i++) dst.AllowedDOFs[i] = src.AllowedDOFs[i];
            dst.IsSensor = src.IsSensor;
        }

        // Copy BoxCollider3DComponent
        if (srcRegistry.all_of<BoxCollider3DComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<BoxCollider3DComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<BoxCollider3DComponent>();
            dst.Offset = src.Offset;
            dst.HalfExtent = src.HalfExtent;
            dst.Friction = src.Friction;
            dst.Restitution = src.Restitution;
        }

        // Copy SphereCollider3DComponent
        if (srcRegistry.all_of<SphereCollider3DComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<SphereCollider3DComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<SphereCollider3DComponent>();
            dst.Offset = src.Offset;
            dst.Radius = src.Radius;
            dst.Friction = src.Friction;
            dst.Restitution = src.Restitution;
        }

        // Copy CapsuleCollider3DComponent
        if (srcRegistry.all_of<CapsuleCollider3DComponent>(srcEntity))
        {
            auto& src = srcRegistry.get<CapsuleCollider3DComponent>(srcEntity);
            auto& dst = newEntity.AddComponent<CapsuleCollider3DComponent>();
            dst.Offset = src.Offset;
            dst.Radius = src.Radius;
            dst.HalfHeight = src.HalfHeight;
            dst.Friction = src.Friction;
            dst.Restitution = src.Restitution;
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

    {
        auto view = m_registry.view<AnimatorComponent>();
        for (auto entity : view)
        {
            auto& anim = view.get<AnimatorComponent>(entity);
            if (anim.IsPlaying && anim.Animator)
            {
                anim.Animator->UpdateAnimation(ts * anim.PlaybackSpeed);
            }
        }
    }

    Renderer2D::BeginScene(camera);
    RenderSprites(ts);
    RenderLightIcons(camera);
    Renderer2D::EndScene();

    auto lights = RD3d::LightSystem::ExtractLights(m_registry);
    Renderer3D::BeginScene(camera, lights);
    Renderer3D::DrawGrid();
    RenderModels(ts);
    Renderer3D::EndScene();
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

void Scene::RenderLightIcons(EditorCamera& camera)
{
    if (!GetShowLightIcons()) return;

    if (!m_DirLightIcon)
    {
        std::string dirPath = (FileSystem::Get().GetRootPath() / "assets/icon/directionLight.png").string();
        m_DirLightIcon = Texture2D::Create(dirPath);
    }
    if (!m_PointLightIcon)
    {
        std::string pointPath = (FileSystem::Get().GetRootPath() / "assets/icon/pointLight.png").string();
        m_PointLightIcon = Texture2D::Create(pointPath);
    }

    auto rotation = glm::mat4(camera.GetOrientation());

    auto dirLightView = m_registry.view<TransformComponent, DirectionalLightComponent>();
    for (auto entity : dirLightView)
    {
        auto [transform, light] = dirLightView.get<TransformComponent, DirectionalLightComponent>(entity);
        glm::mat4 billTransform = glm::translate(glm::mat4(1.0f), transform.Translation) * rotation * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        Renderer2D::DrawQuad(billTransform, m_DirLightIcon, 1.0f, glm::vec4(1.0f), (int)entity);
    }
    
    auto pointLightView = m_registry.view<TransformComponent, PointLightComponent>();
    for (auto entity : pointLightView)
    {
        auto [transform, light] = pointLightView.get<TransformComponent, PointLightComponent>(entity);
        glm::mat4 billTransform = glm::translate(glm::mat4(1.0f), transform.Translation) * rotation * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        Renderer2D::DrawQuad(billTransform, m_PointLightIcon, 1.0f, glm::vec4(1.0f), (int)entity);
    }
}

void Scene::RenderModels(Timestep ts)
{
    auto view = m_registry.view<TransformComponent, Model3DComponent>();
    for (auto entity : view)
    {
        auto [transform, model] = view.get<TransformComponent, Model3DComponent>(entity);
        if (model.IsLoaded)
        {
            const RD3d::Animator* animator = nullptr;
            if (m_registry.all_of<AnimatorComponent>(entity))
            {
                auto& animComp = m_registry.get<AnimatorComponent>(entity);
                if (animComp.Animator)
                    animator = animComp.Animator.get();
            }
            Renderer3D::SubmitModel(*model.ModelData, transform.GetTransform(), (int)entity, animator);
        }
    }
}

void Scene::OnUpdateRuntime(Timestep ts)
{

    {
        auto view = m_registry.view<AnimatorComponent>();
        for (auto entity : view)
        {
            auto& anim = view.get<AnimatorComponent>(entity);
            if (anim.IsPlaying && anim.Animator)
            {
                anim.Animator->UpdateAnimation(ts * anim.PlaybackSpeed);
            }
        }
    }

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

        // 3D Physics Jolt
        m_PhysicsSystem3D.Update(ts);
        JPH::BodyInterface* bodyInterface = m_PhysicsSystem3D.GetBodyInterface();

        auto view3d = m_registry.view<TransformComponent, RigidBody3DComponent>();
        for (auto entity : view3d)
        {
            auto [transform, rb3d] = view3d.get<TransformComponent, RigidBody3DComponent>(entity);

            if (rb3d.RuntimeBodyID == 0xFFFFFFFF)
                continue;

            JPH::BodyID bodyID(rb3d.RuntimeBodyID);
            if (!bodyInterface->IsActive(bodyID))
                continue;

            JPH::Vec3 position = bodyInterface->GetPosition(bodyID);
            JPH::Quat rotation = bodyInterface->GetRotation(bodyID);

            transform.Translation = glm::vec3(position.GetX(), position.GetY(), position.GetZ());
            // glm::quat constructor is (w, x, y, z)
            transform.Rotation = glm::eulerAngles(glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()));
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
        
        auto lights = RD3d::LightSystem::ExtractLights(m_registry);
        Renderer3D::BeginScene(*mainCamera, cameraTransform, lights);

        RenderModels(ts);
        Renderer3D::EndScene();
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

    // --- 3D Physics (Jolt) ---
    m_PhysicsSystem3D.InitializeScene();
    JPH::BodyInterface* bodyInterface = m_PhysicsSystem3D.GetBodyInterface();

    auto view3d = m_registry.view<TransformComponent, RigidBody3DComponent>();
    for (auto entity : view3d)
    {
        auto [transform, rb3d] = view3d.get<TransformComponent, RigidBody3DComponent>(entity);

        JPH::ShapeRefC shape = nullptr;
        float friction = 0.2f;
        float restitution = 0.0f;

        // Determine shape
        if (m_registry.all_of<BoxCollider3DComponent>(entity))
        {
            auto& bc = m_registry.get<BoxCollider3DComponent>(entity);
            glm::vec3 scaleExtents = bc.HalfExtent * transform.Scale;
            shape = new JPH::BoxShape(JPH::Vec3(scaleExtents.x, scaleExtents.y, scaleExtents.z));
            friction = bc.Friction;
            restitution = bc.Restitution;
        }
        else if (m_registry.all_of<SphereCollider3DComponent>(entity))
        {
            auto& sc = m_registry.get<SphereCollider3DComponent>(entity);
            float maxScale = std::max(std::max(transform.Scale.x, transform.Scale.y), transform.Scale.z);
            shape = new JPH::SphereShape(sc.Radius * maxScale);
            friction = sc.Friction;
            restitution = sc.Restitution;
        }
        else if (m_registry.all_of<CapsuleCollider3DComponent>(entity))
        {
            auto& cc = m_registry.get<CapsuleCollider3DComponent>(entity);
            float maxScale = std::max(transform.Scale.x, transform.Scale.z);
            shape = new JPH::CapsuleShape(cc.HalfHeight * transform.Scale.y, cc.Radius * maxScale);
            friction = cc.Friction;
            restitution = cc.Restitution;
        }

        if (shape)
        {
            JPH::EMotionType motionType = JPH::EMotionType::Static;
            JPH::ObjectLayer layer = 0; // Layers::NON_MOVING

            if (rb3d.Type == RigidBody3DComponent::BodyType::Dynamic)
            {
                motionType = JPH::EMotionType::Dynamic;
                layer = 1; // Layers::MOVING
            }
            else if (rb3d.Type == RigidBody3DComponent::BodyType::Kinematic)
            {
                motionType = JPH::EMotionType::Kinematic;
                layer = 1; // Layers::MOVING
            }

            glm::quat q = glm::quat(transform.Rotation);
            JPH::BodyCreationSettings bodySettings(
                shape, 
                JPH::Vec3(transform.Translation.x, transform.Translation.y, transform.Translation.z),
                JPH::Quat(q.x, q.y, q.z, q.w), 
                motionType, 
                layer
            );

            bodySettings.mFriction = friction;
            bodySettings.mRestitution = restitution;
            bodySettings.mLinearDamping = rb3d.LinearDamping;
            bodySettings.mAngularDamping = rb3d.AngularDamping;
            bodySettings.mIsSensor = rb3d.IsSensor;
            
            if (rb3d.Type == RigidBody3DComponent::BodyType::Dynamic)
            {
                bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
                bodySettings.mMassPropertiesOverride.mMass = rb3d.Mass;
            }

            JPH::Body* body = bodyInterface->CreateBody(bodySettings);
            if (body)
            {
                bodyInterface->AddBody(body->GetID(), JPH::EActivation::Activate);
                rb3d.RuntimeBodyID = body->GetID().GetIndexAndSequenceNumber();
            }
        }
    }
}

void Scene::OnRuntimeStop()
{
    if (b2World_IsValid(m_PhysicsWorldId))
    {
        b2DestroyWorld(m_PhysicsWorldId);
        m_PhysicsWorldId = b2_nullWorldId;
    }

    // 3D Physics
    m_PhysicsSystem3D.ShutdownScene();
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

template <typename T> void Scene::OnComponentAdded(Entity entity, T& component)
{
    static_assert(sizeof(T) == 0, "Only specialized components can be added!");
}

template <> void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {}
template <> void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
{
    component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
}

template <> void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) {}
template <> void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) {}
template <> void Scene::OnComponentAdded<SpriteAnimationComponent>(Entity entity, SpriteAnimationComponent& component)
{
}
template <> void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) {}
template <> void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component) {}
template <> void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component) {}
template <> void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) {};
template <> void Scene::OnComponentAdded<Model3DComponent>(Entity entity, Model3DComponent& component) {};
template <> void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component) {};
template <> void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component) {};
template <> void Scene::OnComponentAdded<AnimatorComponent>(Entity entity, AnimatorComponent& component) {};
template <> void Scene::OnComponentAdded<RigidBody3DComponent>(Entity entity, RigidBody3DComponent& component) {};
template <> void Scene::OnComponentAdded<BoxCollider3DComponent>(Entity entity, BoxCollider3DComponent& component) {};
template <> void Scene::OnComponentAdded<SphereCollider3DComponent>(Entity entity, SphereCollider3DComponent& component) {};
template <> void Scene::OnComponentAdded<CapsuleCollider3DComponent>(Entity entity, CapsuleCollider3DComponent& component) {};

} // namespace UHE
