#pragma once
#include <fastgltf/core.hpp>
#include <filesystem>
#include "fastgltf/types.hpp"
#include "UHE/Renderer/Texture.h"
#include "UHE/Renderer3D/Animation.h"

#include "UHE/RHI/RHITypes.h"

namespace UHE::RD3d
{

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 jointIndices = glm::ivec4(0);
    glm::vec4 jointWeights = glm::vec4(0.0f);
};

struct Material
{
    Ref<Texture2D> AlbedoTexture = nullptr;
    Ref<Texture2D> MetallicRoughnessTexture = nullptr;
    float MetallicFactor = 1.0f;
    float RoughnessFactor = 1.0f;
};

struct Primitive
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    size_t materialIndex = 0;

    // RHI resources
    RHI::BufferHandle VertexBuffer = nullptr;
    RHI::BufferHandle IndexBuffer = nullptr;
    uint32_t IndexCount = 0;
};

struct Mesh
{
    std::string name;
    std::vector<Primitive> primitive;
};

class UHE_API Model
{
public:
    Model() = default;
    ~Model();
    bool loadModel(const std::filesystem::path& filepath);
    void Destroy();

    const std::vector<Mesh>& GetMesh() const { return m_LoadedMeshes; }
    const std::vector<Material>& GetMaterials() const { return m_LoadedMaterials; }

private:
    void ProcessNode(const fastgltf::Asset& asset, size_t nodeIndex);
    void ExtractMesh(const fastgltf::Asset& asset, const fastgltf::Mesh& gltfMesh);
    void ParseSkins(const fastgltf::Asset& asset);
    void ParseAnimations(const fastgltf::Asset& asset);

private:
    std::vector<Mesh> m_LoadedMeshes;
    std::vector<Material> m_LoadedMaterials;

    Skeleton m_Skeleton;
    std::vector<AnimationClip> m_Animations;

public:
    const Skeleton& GetSkeleton() const { return m_Skeleton; }
    const std::vector<AnimationClip>& GetAnimations() const { return m_Animations; }
};

} // namespace UHE::RD3d
