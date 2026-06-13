#include "uhepch.h"
#include "LoadModel.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include "UHE/RHI/RHICommadBuffer.h"
#include "UHE/Renderer/Renderer.h"
#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include "glm/ext/vector_float3.hpp"

namespace UHE::RD3d
{

Model::~Model()
{
    Destroy();
}

void Model::Destroy()
{
    auto& device = Renderer::GetDevice();
    for (auto& mesh : m_LoadedMeshes)
    {
        for (auto& prim : mesh.primitive)
        {
            if (prim.VertexBuffer)
            {
                device.DestroyBuffer(prim.VertexBuffer);
                prim.VertexBuffer = nullptr;
            }
            if (prim.IndexBuffer)
            {
                device.DestroyBuffer(prim.IndexBuffer);
                prim.IndexBuffer = nullptr;
            }
        }
    }
}

bool Model::loadModel(const std::filesystem::path& filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        UHE_CORE_ERROR("File not Found {0}", filepath.string());
        return false;
    }

    Destroy();
    m_LoadedMeshes.clear();
    m_LoadedMaterials.clear();

    static constexpr auto supportedExtensions = ~fastgltf::Extensions::None;
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
        return false;
    }

    fastgltf::Asset asset = std::move(assetResult.get());

    m_LoadedMaterials.resize(asset.materials.size());
    for (size_t i = 0; i < asset.materials.size(); ++i)
    {
        auto& gltfMaterial = asset.materials[i];
        const fastgltf::TextureInfo* albedoTextureInfo = nullptr;
        if (gltfMaterial.pbrData.baseColorTexture.has_value())
        {
            albedoTextureInfo = &gltfMaterial.pbrData.baseColorTexture.value();
        }
        else if (gltfMaterial.specularGlossiness && gltfMaterial.specularGlossiness->diffuseTexture.has_value())
        {
            albedoTextureInfo = &gltfMaterial.specularGlossiness->diffuseTexture.value();
        }

        if (albedoTextureInfo)
        {
            auto textureIndex = albedoTextureInfo->textureIndex;
            auto imageIndex = asset.textures[textureIndex].imageIndex;
            if (imageIndex.has_value())
            {
                auto& image = asset.images[imageIndex.value()];
                std::visit(
                    fastgltf::visitor{[&](const fastgltf::sources::URI& filePath)
                                      {
                                          std::filesystem::path imgPath = filepath.parent_path() / filePath.uri.path();
                                          m_LoadedMaterials[i].AlbedoTexture = Texture2D::Create(imgPath.string());
                                      },
                                      [&](const fastgltf::sources::Array& array)
                                      {
                                          m_LoadedMaterials[i].AlbedoTexture =
                                              Texture2D::CreateFromMemory(array.bytes.data(), array.bytes.size());
                                          UHE_CORE_INFO("Loaded texture for material {0} from Array, size: {1}", i,
                                                        array.bytes.size());
                                      },
                                      [&](const fastgltf::sources::ByteView& byteView)
                                      {
                                          m_LoadedMaterials[i].AlbedoTexture =
                                              Texture2D::CreateFromMemory(byteView.bytes.data(), byteView.bytes.size());
                                          UHE_CORE_INFO("Loaded texture for material {0} from ByteView, size: {1}", i,
                                                        byteView.bytes.size());
                                      },
                                      [&](const fastgltf::sources::Vector& vector)
                                      {
                                          m_LoadedMaterials[i].AlbedoTexture =
                                              Texture2D::CreateFromMemory(vector.bytes.data(), vector.bytes.size());
                                          UHE_CORE_INFO("Loaded texture for material {0} from Vector, size: {1}", i,
                                                        vector.bytes.size());
                                      },
                                      [&](const fastgltf::sources::Fallback& fallback)
                                      {
                                          UHE_CORE_ERROR("fastgltf fallback triggered for material {0} image! Image could not be loaded.", i);
                                      },
                                      [&](const fastgltf::sources::BufferView& view)
                                      {
                                          auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                                          auto& buffer = asset.buffers[bufferView.bufferIndex];
                                          std::visit(
                                              fastgltf::visitor{
                                                  [&](const fastgltf::sources::Array& array)
                                                  {
                                                      const void* data = array.bytes.data() + bufferView.byteOffset;
                                                      m_LoadedMaterials[i].AlbedoTexture =
                                                          Texture2D::CreateFromMemory(data, bufferView.byteLength);
                                                      UHE_CORE_INFO(
                                                          "Loaded texture for material {0} from BufferView (Array), size: {1}",
                                                          i, bufferView.byteLength);
                                                  },
                                                  [&](const fastgltf::sources::ByteView& byteView)
                                                  {
                                                      const void* data = byteView.bytes.data() + bufferView.byteOffset;
                                                      m_LoadedMaterials[i].AlbedoTexture =
                                                          Texture2D::CreateFromMemory(data, bufferView.byteLength);
                                                      UHE_CORE_INFO(
                                                          "Loaded texture for material {0} from BufferView (ByteView), size: {1}",
                                                          i, bufferView.byteLength);
                                                  },
                                                  [&](const fastgltf::sources::Vector& vector)
                                                  {
                                                      const void* data = vector.bytes.data() + bufferView.byteOffset;
                                                      m_LoadedMaterials[i].AlbedoTexture =
                                                          Texture2D::CreateFromMemory(data, bufferView.byteLength);
                                                      UHE_CORE_INFO(
                                                          "Loaded texture for material {0} from BufferView (Vector), size: {1}",
                                                          i, bufferView.byteLength);
                                                  },
                                                  [&](const auto&) {
                                                      UHE_CORE_ERROR("Unhandled buffer data type in glTF! Variant index: {0}", buffer.data.index());
                                                  }},
                                              buffer.data);
                                      },
                                      [&](const auto&) {
                                          UHE_CORE_ERROR("Unhandled image data type in glTF! Variant index: {0}", image.data.index());
                                      }},
                    image.data);
            }
        }
    }

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
    outMesh.name = gltfMesh.name.empty() ? "Unnamed_Mesh" : std::string(gltfMesh.name);

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
                                                                      outPrim.vertices[idx].normal =
                                                                          glm::vec3(0.0f, 1.0f, 0.0f); // Default
                                                                      outPrim.vertices[idx].uv = glm::vec2(0.0f);
                                                                  });

        const auto* normalAttribute = primitive.findAttribute("NORMAL");
        if (normalAttribute != primitive.attributes.end())
        {
            auto& normalAccessor = asset.accessors[normalAttribute->accessorIndex];
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
                asset, normalAccessor, [&](fastgltf::math::fvec3 norm, size_t idx)
                { outPrim.vertices[idx].normal = glm::vec3(norm.x(), norm.y(), norm.z()); });
        }

        const auto* uvAttribute = primitive.findAttribute("TEXCOORD_0");
        if (uvAttribute != primitive.attributes.end())
        {
            auto& uvAccessor = asset.accessors[uvAttribute->accessorIndex];

            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
                asset, uvAccessor,
                [&](fastgltf::math::fvec2 uv, size_t idx) { outPrim.vertices[idx].uv = glm::vec2(uv.x(), 1.0f - uv.y()); });
        }

        if (primitive.indicesAccessor.has_value())
        {
            auto& indicesAccessor = asset.accessors[primitive.indicesAccessor.value()];
            outPrim.indices.reserve(indicesAccessor.count);

            fastgltf::iterateAccessor<u32>(asset, indicesAccessor,
                                           [&](u32 indexValue) { outPrim.indices.push_back(indexValue); });
        }

        auto& device = Renderer::GetDevice();
        auto& cmd = device.GetCurrentCommandBuffer();

        // 1. Create and Upload Vertex Buffer
        if (!outPrim.vertices.empty())
        {
            RHI::BufferDesc vbDesc{};
            vbDesc.size = outPrim.vertices.size() * sizeof(Vertex);
            vbDesc.usage = RHI::BufferUsage::Vertex;
            vbDesc.hostVisible = true;
            outPrim.VertexBuffer = device.CreateBuffer(vbDesc);
            cmd.UpdateBuffer(outPrim.VertexBuffer, outPrim.vertices.data(), vbDesc.size);
        }

        // 2. Create and Upload Index Buffer
        if (!outPrim.indices.empty())
        {
            outPrim.IndexCount = static_cast<uint32_t>(outPrim.indices.size());
            RHI::BufferDesc ibDesc{};
            ibDesc.size = outPrim.indices.size() * sizeof(u32);
            ibDesc.usage = RHI::BufferUsage::Index;
            ibDesc.hostVisible = true;
            outPrim.IndexBuffer = device.CreateBuffer(ibDesc);
            cmd.UpdateBuffer(outPrim.IndexBuffer, outPrim.indices.data(), ibDesc.size);
        }

        outMesh.primitive.push_back(std::move(outPrim));
    }
    m_LoadedMeshes.push_back(std::move(outMesh));
}
} // namespace UHE::RD3d
