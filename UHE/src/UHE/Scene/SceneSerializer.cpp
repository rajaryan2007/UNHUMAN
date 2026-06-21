#include "uhepch.h"
#include "SceneSerializer.h"
#include <fstream>
#include "Components.h"
#include "Entity.h"
#include "UHE/Renderer/Texture.h"
#include "UHE/Utils/YamlUtils.h"

namespace UHE
{

SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : m_Scene(scene) {}

static void SerializeEntity(YAML::Emitter& out, Entity entity)
{
    UHE_CORE_ASSERT(entity, "Entity is null!");
    UHE_CORE_ASSERT(entity.GetUUID() != 0, "Entity must have a valid ID!");

    out << YAML::BeginMap;

    out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID(); // TODO enitity

    if (entity.HasComponent<TagComponent>())
    {
        out << YAML::Key << "TagComponent" << YAML::BeginMap;
        out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<TagComponent>().Tag;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<TransformComponent>())
    {
        auto& tc = entity.GetComponent<TransformComponent>();

        out << YAML::Key << "TransformComponent" << YAML::BeginMap;
        out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
        out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
        out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
        out << YAML::EndMap;
    }

    if (entity.HasComponent<CameraComponent>())
    {
        auto& cc = entity.GetComponent<CameraComponent>();
        auto& cam = cc.Camera;

        out << YAML::Key << "CameraComponent" << YAML::BeginMap;

        out << YAML::Key << "ProjectionType" << YAML::Value << (int)cam.GetProjectionType();
        out << YAML::Key << "PerspectiveFOV" << YAML::Value << cam.GetProjectionFOV();
        out << YAML::Key << "PerspectiveNear" << YAML::Value << cam.GetProjectionNear();
        out << YAML::Key << "PerspectiveFar" << YAML::Value << cam.GetProjectionFar();

        out << YAML::Key << "OrthographicSize" << YAML::Value << cam.GetOrthographicSize();
        out << YAML::Key << "OrthographicNear" << YAML::Value << cam.GetOrthographicNearClip();
        out << YAML::Key << "OrthographicFar" << YAML::Value << cam.GetOrthographicFarClip();

        out << YAML::Key << "Primary" << YAML::Value << cc.Primary;
        out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.FixedAspectRatio;

        out << YAML::EndMap;
    }

    // Sprite
    if (entity.HasComponent<SpriteRendererComponent>())
    {
        auto& src = entity.GetComponent<SpriteRendererComponent>();

        out << YAML::Key << "SpriteRendererComponent" << YAML::BeginMap;
        out << YAML::Key << "Color" << YAML::Value << src.Color;
        out << YAML::Key << "TexturePath" << YAML::Value << src.TexturePath;
        out << YAML::Key << "TilingFactor" << YAML::Value << src.TilingFactor;
        out << YAML::Key << "UseSubTexture" << YAML::Value << src.UseSubTexture;
        out << YAML::Key << "SubTextureCoords" << YAML::Value << src.SubTextureCoords;
        out << YAML::Key << "SubTextureCellSize" << YAML::Value << src.SubTextureCellSize;
        out << YAML::Key << "SubTextureSpriteSize" << YAML::Value << src.SubTextureSpriteSize;
        out << YAML::EndMap;
    }

    // Sprite Animation
    if (entity.HasComponent<SpriteAnimationComponent>())
    {
        out << YAML::Key << "SpriteAnimationComponent";
        out << YAML::BeginMap;

        auto& animComp = entity.GetComponent<SpriteAnimationComponent>();
        auto& anim = animComp.Animation;

        out << YAML::Key << "SpriteSheetPath" << YAML::Value << animComp.SpriteSheetPath;
        out << YAML::Key << "FrameSizeX" << YAML::Value << anim.FrameSize.x;
        out << YAML::Key << "FrameSizeY" << YAML::Value << anim.FrameSize.y;
        out << YAML::Key << "FrameCount" << YAML::Value << anim.FrameCount;
        out << YAML::Key << "FrameDuration" << YAML::Value << anim.FrameDuration;
        out << YAML::Key << "Loop" << YAML::Value << anim.Loop;
        out << YAML::Key << "Color" << YAML::Value << animComp.Color;

        out << YAML::EndMap; // SpriteAnimationComponent
    }

    // RigidBody2D
    if (entity.HasComponent<RigidBody2DComponent>())
    {
        auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

        out << YAML::Key << "RigidBody2DComponent" << YAML::BeginMap;
        out << YAML::Key << "BodyType" << YAML::Value << (int)rb2d.Type;
        out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;
        out << YAML::EndMap;
    }

    // BoxCollider2D
    if (entity.HasComponent<BoxColliderComponent>())
    {
        auto& bc2d = entity.GetComponent<BoxColliderComponent>();

        out << YAML::Key << "BoxColliderComponent" << YAML::BeginMap;
        out << YAML::Key << "Offset" << YAML::Value << bc2d.Offset;
        out << YAML::Key << "Size" << YAML::Value << bc2d.Size;
        out << YAML::Key << "Density" << YAML::Value << bc2d.Density;
        out << YAML::Key << "Friction" << YAML::Value << bc2d.Friction;
        out << YAML::Key << "Restitution" << YAML::Value << bc2d.Restitution;
        out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2d.RestitutionThreshold;
        out << YAML::EndMap;
    }

    // RigidBody3D
    if (entity.HasComponent<RigidBody3DComponent>())
    {
        auto& rb3d = entity.GetComponent<RigidBody3DComponent>();

        out << YAML::Key << "RigidBody3DComponent" << YAML::BeginMap;
        out << YAML::Key << "BodyType" << YAML::Value << (int)rb3d.Type;
        out << YAML::Key << "Mass" << YAML::Value << rb3d.Mass;
        out << YAML::Key << "LinearDamping" << YAML::Value << rb3d.LinearDamping;
        out << YAML::Key << "AngularDamping" << YAML::Value << rb3d.AngularDamping;
        out << YAML::Key << "IsSensor" << YAML::Value << rb3d.IsSensor;
        out << YAML::Key << "AllowedDOFs" << YAML::BeginSeq;
        for (int i = 0; i < 6; i++) out << rb3d.AllowedDOFs[i];
        out << YAML::EndSeq;
        out << YAML::EndMap;
    }

    // BoxCollider3D
    if (entity.HasComponent<BoxCollider3DComponent>())
    {
        auto& bc3d = entity.GetComponent<BoxCollider3DComponent>();

        out << YAML::Key << "BoxCollider3DComponent" << YAML::BeginMap;
        out << YAML::Key << "Offset" << YAML::Value << bc3d.Offset;
        out << YAML::Key << "HalfExtent" << YAML::Value << bc3d.HalfExtent;
        out << YAML::Key << "Friction" << YAML::Value << bc3d.Friction;
        out << YAML::Key << "Restitution" << YAML::Value << bc3d.Restitution;
        out << YAML::EndMap;
    }

    // SphereCollider3D
    if (entity.HasComponent<SphereCollider3DComponent>())
    {
        auto& sc3d = entity.GetComponent<SphereCollider3DComponent>();

        out << YAML::Key << "SphereCollider3DComponent" << YAML::BeginMap;
        out << YAML::Key << "Offset" << YAML::Value << sc3d.Offset;
        out << YAML::Key << "Radius" << YAML::Value << sc3d.Radius;
        out << YAML::Key << "Friction" << YAML::Value << sc3d.Friction;
        out << YAML::Key << "Restitution" << YAML::Value << sc3d.Restitution;
        out << YAML::EndMap;
    }

    // CapsuleCollider3D
    if (entity.HasComponent<CapsuleCollider3DComponent>())
    {
        auto& cc3d = entity.GetComponent<CapsuleCollider3DComponent>();

        out << YAML::Key << "CapsuleCollider3DComponent" << YAML::BeginMap;
        out << YAML::Key << "Offset" << YAML::Value << cc3d.Offset;
        out << YAML::Key << "Radius" << YAML::Value << cc3d.Radius;
        out << YAML::Key << "HalfHeight" << YAML::Value << cc3d.HalfHeight;
        out << YAML::Key << "Friction" << YAML::Value << cc3d.Friction;
        out << YAML::Key << "Restitution" << YAML::Value << cc3d.Restitution;
        out << YAML::EndMap;
    }

    // Model3D
    if (entity.HasComponent<Model3DComponent>())
    {
        out << YAML::Key << "Model3DComponent" << YAML::BeginMap;
        auto& mc = entity.GetComponent<Model3DComponent>();
        out << YAML::Key << "ModelPath" << YAML::Value << mc.ModelPath;
        out << YAML::EndMap;
    }

    // Animator
    if (entity.HasComponent<AnimatorComponent>())
    {
        auto& ac = entity.GetComponent<AnimatorComponent>();
        out << YAML::Key << "AnimatorComponent" << YAML::BeginMap;
        out << YAML::Key << "CurrentAnimationName" << YAML::Value << ac.CurrentAnimationName;
        out << YAML::Key << "PlaybackSpeed" << YAML::Value << ac.PlaybackSpeed;
        out << YAML::Key << "IsPlaying" << YAML::Value << ac.IsPlaying;
        out << YAML::EndMap;
    }

    // DirectionalLight
    if (entity.HasComponent<DirectionalLightComponent>())
    {
        auto& dlc = entity.GetComponent<DirectionalLightComponent>();
        out << YAML::Key << "DirectionalLightComponent" << YAML::BeginMap;
        out << YAML::Key << "Color" << YAML::Value << dlc.Color;
        out << YAML::Key << "Intensity" << YAML::Value << dlc.Intensity;
        out << YAML::EndMap;
    }

    // PointLight
    if (entity.HasComponent<PointLightComponent>())
    {
        auto& plc = entity.GetComponent<PointLightComponent>();
        out << YAML::Key << "PointLightComponent" << YAML::BeginMap;
        out << YAML::Key << "Color" << YAML::Value << plc.Color;
        out << YAML::Key << "Intensity" << YAML::Value << plc.Intensity;
        out << YAML::Key << "Radius" << YAML::Value << plc.Radius;
        out << YAML::EndMap;
    }

    out << YAML::EndMap;
}

void SceneSerializer::Serialize(const std::string& filepath)
{
    YAML::Emitter out;

    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << "Untitled";
    out << YAML::Key << "Entities" << YAML::BeginSeq;

    auto view = m_Scene->m_registry.view<entt::entity>();
    for (auto entityID : view)
    {
        Entity entity{entityID, m_Scene.get()};
        if (entity)
            SerializeEntity(out, entity);
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;

    std::ofstream fout(filepath);
    fout << out.c_str();
}

bool SceneSerializer::Deserialize(const std::string& filepath)
{
    std::ifstream stream(filepath);
    if (!stream.is_open())
        return false;

    YAML::Node data = YAML::Load(stream);
    if (!data["Scene"])
        return false;

    auto entities = data["Entities"];
    if (!entities)
        return true;

    for (auto entityNode : entities)
    {
        std::string name = "Entity";

        auto tagNode = entityNode["TagComponent"];
        if (tagNode)
            name = tagNode["Tag"].as<std::string>();

        Entity entity = m_Scene->CreateEntity(name);

        // Transform
        auto transformNode = entityNode["TransformComponent"];
        if (transformNode)
        {
            auto& tc = entity.GetComponent<TransformComponent>();
            tc.Translation = transformNode["Translation"].as<glm::vec3>();
            tc.Rotation = transformNode["Rotation"].as<glm::vec3>();
            tc.Scale = transformNode["Scale"].as<glm::vec3>();
        }

        auto cameraNode = entityNode["CameraComponent"];
        if (cameraNode)
        {
            auto& cc = entity.AddComponent<CameraComponent>();
            auto& cam = cc.Camera;

            cam.SetProjectionType((SceneCamera::ProjectionType)cameraNode["ProjectionType"].as<int>());

            cam.SetPerspectiveFOV(cameraNode["PerspectiveFOV"].as<float>());
            cam.SetPerpecstiveNear(cameraNode["PerspectiveNear"].as<float>());
            cam.SetPerspecstiveFar(cameraNode["PerspectiveFar"].as<float>());

            cam.SetOrthographicSize(cameraNode["OrthographicSize"].as<float>());
            cam.SetOrthoGraphicNearClip(cameraNode["OrthographicNear"].as<float>());
            cam.SetOrthoGraphicFarClip(cameraNode["OrthographicFar"].as<float>());

            cc.Primary = cameraNode["Primary"].as<bool>();
            cc.FixedAspectRatio = cameraNode["FixedAspectRatio"].as<bool>();
        }

        auto spriteNode = entityNode["SpriteRendererComponent"];
        if (spriteNode)
        {
            auto& src = entity.AddComponent<SpriteRendererComponent>();
            src.Color = spriteNode["Color"].as<glm::vec4>();

            if (spriteNode["TexturePath"])
            {
                src.TexturePath = spriteNode["TexturePath"].as<std::string>();
                if (!src.TexturePath.empty())
                {
                    src.Texture = Texture2D::Create(src.TexturePath);
                }
            }

            if (spriteNode["TilingFactor"])
                src.TilingFactor = spriteNode["TilingFactor"].as<float>();

            if (spriteNode["UseSubTexture"])
            {
                src.UseSubTexture = spriteNode["UseSubTexture"].as<bool>();
                src.SubTextureCoords = spriteNode["SubTextureCoords"].as<glm::vec2>();
                src.SubTextureCellSize = spriteNode["SubTextureCellSize"].as<glm::vec2>();
                src.SubTextureSpriteSize = spriteNode["SubTextureSpriteSize"].as<glm::vec2>();
            }
        }

        auto animNode = entityNode["SpriteAnimationComponent"];
        if (animNode)
        {
            auto& animComp = entity.AddComponent<SpriteAnimationComponent>();
            animComp.SpriteSheetPath = animNode["SpriteSheetPath"].as<std::string>();
            auto& anim = animComp.Animation;
            if (!animComp.SpriteSheetPath.empty())
            {
                anim.SpriteSheet = Texture2D::Create(animComp.SpriteSheetPath);
            }

            anim.FrameSize.x = animNode["FrameSizeX"].as<float>();
            anim.FrameSize.y = animNode["FrameSizeY"].as<float>();
            anim.FrameCount = animNode["FrameCount"].as<int>();
            anim.FrameDuration = animNode["FrameDuration"].as<float>();
            anim.Loop = animNode["Loop"].as<bool>();
            if (animNode["Color"])
                animComp.Color = animNode["Color"].as<glm::vec4>();
        }

        // RigidBody2D
        auto rb2dNode = entityNode["RigidBody2DComponent"];
        if (rb2dNode)
        {
            auto& rb2d = entity.AddComponent<RigidBody2DComponent>();
            rb2d.Type = (RigidBody2DComponent::BodyType)rb2dNode["BodyType"].as<int>();
            if (rb2dNode["FixedRotation"])
                rb2d.FixedRotation = rb2dNode["FixedRotation"].as<bool>();
        }

        // BoxCollider2D
        auto bc2dNode = entityNode["BoxColliderComponent"];
        if (bc2dNode)
        {
            auto& bc2d = entity.AddComponent<BoxColliderComponent>();
            bc2d.Offset = bc2dNode["Offset"].as<glm::vec2>();
            bc2d.Size = bc2dNode["Size"].as<glm::vec2>();
            bc2d.Density = bc2dNode["Density"].as<float>();
            bc2d.Friction = bc2dNode["Friction"].as<float>();
            bc2d.Restitution = bc2dNode["Restitution"].as<float>();
            if (bc2dNode["RestitutionThreshold"])
                bc2d.RestitutionThreshold = bc2dNode["RestitutionThreshold"].as<float>();
        }

        // RigidBody3D
        auto rb3dNode = entityNode["RigidBody3DComponent"];
        if (rb3dNode)
        {
            auto& rb3d = entity.AddComponent<RigidBody3DComponent>();
            rb3d.Type = (RigidBody3DComponent::BodyType)rb3dNode["BodyType"].as<int>();
            if (rb3dNode["Mass"]) rb3d.Mass = rb3dNode["Mass"].as<float>();
            if (rb3dNode["LinearDamping"]) rb3d.LinearDamping = rb3dNode["LinearDamping"].as<float>();
            if (rb3dNode["AngularDamping"]) rb3d.AngularDamping = rb3dNode["AngularDamping"].as<float>();
            if (rb3dNode["IsSensor"]) rb3d.IsSensor = rb3dNode["IsSensor"].as<bool>();
            if (rb3dNode["AllowedDOFs"] && rb3dNode["AllowedDOFs"].IsSequence())
            {
                auto seq = rb3dNode["AllowedDOFs"];
                for (int i = 0; i < 6 && i < seq.size(); i++)
                    rb3d.AllowedDOFs[i] = seq[i].as<bool>();
            }
        }

        // BoxCollider3D
        auto bc3dNode = entityNode["BoxCollider3DComponent"];
        if (bc3dNode)
        {
            auto& bc3d = entity.AddComponent<BoxCollider3DComponent>();
            bc3d.Offset = bc3dNode["Offset"].as<glm::vec3>();
            bc3d.HalfExtent = bc3dNode["HalfExtent"].as<glm::vec3>();
            bc3d.Friction = bc3dNode["Friction"].as<float>();
            bc3d.Restitution = bc3dNode["Restitution"].as<float>();
        }

        // SphereCollider3D
        auto sc3dNode = entityNode["SphereCollider3DComponent"];
        if (sc3dNode)
        {
            auto& sc3d = entity.AddComponent<SphereCollider3DComponent>();
            sc3d.Offset = sc3dNode["Offset"].as<glm::vec3>();
            sc3d.Radius = sc3dNode["Radius"].as<float>();
            sc3d.Friction = sc3dNode["Friction"].as<float>();
            sc3d.Restitution = sc3dNode["Restitution"].as<float>();
        }

        // CapsuleCollider3D
        auto cc3dNode = entityNode["CapsuleCollider3DComponent"];
        if (cc3dNode)
        {
            auto& cc3d = entity.AddComponent<CapsuleCollider3DComponent>();
            cc3d.Offset = cc3dNode["Offset"].as<glm::vec3>();
            cc3d.Radius = cc3dNode["Radius"].as<float>();
            cc3d.HalfHeight = cc3dNode["HalfHeight"].as<float>();
            cc3d.Friction = cc3dNode["Friction"].as<float>();
            cc3d.Restitution = cc3dNode["Restitution"].as<float>();
        }

        // Model3D
        auto modelNode = entityNode["Model3DComponent"];
        if (modelNode)
        {
            auto& mc = entity.AddComponent<Model3DComponent>();
            mc.ModelPath = modelNode["ModelPath"].as<std::string>();
            if (!mc.ModelPath.empty())
            {
                mc.IsLoaded = mc.ModelData->loadModel(mc.ModelPath);
            }
        }

        // Animator
        auto acNode = entityNode["AnimatorComponent"];
        if (acNode)
        {
            auto& ac = entity.AddComponent<AnimatorComponent>();
            ac.CurrentAnimationName = acNode["CurrentAnimationName"].as<std::string>();
            ac.PlaybackSpeed = acNode["PlaybackSpeed"].as<float>();
            ac.IsPlaying = acNode["IsPlaying"].as<bool>();
            if (entity.HasComponent<Model3DComponent>())
            {
                auto& mc = entity.GetComponent<Model3DComponent>();
                if (mc.IsLoaded && mc.ModelData)
                {
                    ac.Animator = CreateRef<RD3d::Animator>(mc.ModelData);
                    if (!ac.CurrentAnimationName.empty())
                        ac.Animator->PlayAnimation(ac.CurrentAnimationName);
                }
            }
        }

        // DirectionalLight
        auto dlcNode = entityNode["DirectionalLightComponent"];
        if (dlcNode)
        {
            auto& dlc = entity.AddComponent<DirectionalLightComponent>();
            dlc.Color = dlcNode["Color"].as<glm::vec3>();
            dlc.Intensity = dlcNode["Intensity"].as<float>();
        }

        // PointLight
        auto plcNode = entityNode["PointLightComponent"];
        if (plcNode)
        {
            auto& plc = entity.AddComponent<PointLightComponent>();
            plc.Color = plcNode["Color"].as<glm::vec3>();
            plc.Intensity = plcNode["Intensity"].as<float>();
            plc.Radius = plcNode["Radius"].as<float>();
        }
    }

    return true;
}

} // namespace UHE
