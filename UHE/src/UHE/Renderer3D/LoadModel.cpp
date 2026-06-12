#include "uhepch.h"
#include "LoadModel.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include "glm/ext/vector_float3.hpp"

namespace UHE::RD3d
{
bool Model::loadModel(const std::filesystem::path& filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        UHE_CORE_ERROR("File not Found {0}", filepath.string());
        return false;
    }

    m_LoadedMeshes.clear();

    static constexpr auto supportedExtensions = fastgltf::Extensions::KHR_mesh_quantization |
                                                fastgltf::Extensions::KHR_texture_transform |
                                                fastgltf::Extensions::KHR_materials_variants;
    fastgltf::Parser parser(supportedExtensions);

    constexpr auto gltfOption = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                                fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages |
                                fastgltf::Options::GenerateMeshIndices;

    auto gltfFile = fastgltf::MappedGltfFile::FromPath(filepath);
    if (!gltfFile)
    {
        UHE_CORE_ERROR("Failed to open/map glTF file: {0}", fastgltf::getErrorMessage(gltfFile.error()));
        return false;
    }

    auto assetResult = parser.loadGltf(gltfFile.get(), filepath.parent_path(), gltfOption);
    if (assetResult.error() != fastgltf::Error::None)
    {
        UHE_CORE_ERROR("Failed to parse glTF data: {0}", fastgltf::getErrorMessage(assetResult.error()));
    }

    fastgltf::Asset asset = std::move(assetResult.get());

    size_t activeSceneIndex = asset.defaultScene.value_or(0);
    if (!asset.scenes.empty() && activeSceneIndex < asset.scenes.size())
    {
        auto& scene = asset.scenes[activeSceneIndex];
        for (auto& rootNodeIndex : scene.nodeIndices)
        {
            ProcessNode(asset, rootNodeIndex);
        }
    }
    return true;
}

void Model::ProcessNode(const fastgltf::Asset& asset, size_t nodeIndex)
{
    auto& node = asset.nodes[nodeIndex];
    if (node.meshIndex.has_value())
    {
        ExtractMesh(asset, asset.meshes[node.meshIndex.value()]);
    }
    for (auto& childIndex : node.children)
    {
        ProcessNode(asset, childIndex);
    }
}

void Model::ExtractMesh(const fastgltf::Asset& asset, const fastgltf::Mesh& gltfMesh)
{
    Mesh outMesh;
    outMesh.name = gltfMesh.name.empty() ? std::string(gltfMesh.name) : "Unnamed_Mesh";

    for (auto& primitive : gltfMesh.primitives)
    {
        Primitive outPrim;
        outPrim.materialIndex = primitive.materialIndex.value_or(0);
        const auto* posAttribute = primitive.findAttribute("POSITION");
        if (posAttribute == primitive.attributes.end())
            continue;

        auto& posAccessor = asset.accessors[posAttribute->accessorIndex];
        outPrim.vertices.resize(posAccessor.count);

        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, posAccessor,
                                                                  [&](fastgltf::math::fvec3 pos, size_t idx)
                                                                  {
                                                                      outPrim.vertices[idx].position =
                                                                          glm::vec3(pos.x(), pos.y(), pos.z());
                                                                      outPrim.vertices[idx].normal = glm::vec3(0.0);
                                                                      outPrim.vertices[idx].uv = glm::vec2(0.0f);
                                                                  });

        const auto* uvAttribute = primitive.findAttribute("TEXCOORD_0");
        if (uvAttribute != primitive.attributes.end())
        {
            auto& uvAccessor = asset.accessors[uvAttribute->accessorIndex];

            fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, uvAccessor, [&](glm::vec2 uv, size_t idx)
                                                          { outPrim.vertices[idx].uv = uv; });
        }

        if (primitive.indicesAccessor.has_value())
        {
            auto& indicesAccessor = asset.accessors[primitive.indicesAccessor.value()];
            outPrim.indices.reserve(indicesAccessor.count);

            fastgltf::iterateAccessor<u32>(asset, indicesAccessor,
                                           [&](u32 indexValue) { outPrim.indices.push_back(indexValue); });
        }
        outMesh.primitive.push_back(std::move(outPrim));
    }
    m_LoadedMeshes.push_back(std::move(outMesh));
}
} // namespace UHE::RD3d
