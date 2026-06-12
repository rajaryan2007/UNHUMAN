#pragma once
#include <fastgltf/core.hpp>
#include <filesystem>
#include "fastgltf/types.hpp"

namespace UHE::RD3d
{

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Primitive
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    size_t materialIndex = 0;
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
    ~Model() = default;
    bool loadModel(const std::filesystem::path& filepath);

    const std::vector<Mesh>& GetMesh() const { return m_LoadedMeshes; }

private:
    void ProcessNode(const fastgltf::Asset& asset, size_t nodeIndex);
    void ExtractMesh(const fastgltf::Asset& asset, const fastgltf::Mesh& gltfMesh);

private:
    std::vector<Mesh> m_LoadedMeshes;
};

} // namespace UHE::RD3d
