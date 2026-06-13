#pragma once
#include <fastgltf/core.hpp>
#include <filesystem>
#include "fastgltf/types.hpp"
#include "UHE/Renderer/Texture.h"

#include "UHE/RHI/RHITypes.h"

namespace UHE::RD3d
{

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Material
{
    Ref<Texture2D> AlbedoTexture = nullptr;
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

class Model
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

private:
    std::vector<Mesh> m_LoadedMeshes;
    std::vector<Material> m_LoadedMaterials;
};

} // namespace UHE::RD3d
