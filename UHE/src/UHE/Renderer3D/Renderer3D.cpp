#include "uhepch.h"
#include "Renderer3D.h"
#include "UHE/Renderer/Renderer.h"
#include "UHE/RHI/RHIDevice.h"
#include "UHE/RHI/RHICommadBuffer.h"
#include "UHE/Renderer/SlangCompiler.h"
#include "UHE/AssestsManager/VfsSystem.h"

namespace UHE
{

struct Renderer3DData
{
    RHI::ShaderHandle VertexShader;
    RHI::ShaderHandle FragmentShader;
    RHI::PipelineHandle ModelPipeline;
    
    RHI::ShaderHandle GridVertexShader;
    RHI::ShaderHandle GridFragmentShader;
    RHI::PipelineHandle GridPipeline;
    
    glm::mat4 ViewProjection;
    Ref<Texture2D> WhiteTexture;
};

static Renderer3DData s_Data3D;

void Renderer3D::Init()
{
    auto& device = Renderer::GetDevice();
    
    std::string shaderPath = (FileSystem::Get().GetRootPath() / "assets/shaders/Basic3D.slang").string();
    auto compiledShaders = SlangCompiler::CompileToSPIRV(shaderPath);

    if (compiledShaders.find(RHI::ShaderStage::Vertex) != compiledShaders.end())
    {
        RHI::ShaderDesc vsDesc{};
        vsDesc.stage = RHI::ShaderStage::Vertex;
        vsDesc.spirvData = (const uint8_t*)compiledShaders[RHI::ShaderStage::Vertex].data();
        vsDesc.spirvSize = compiledShaders[RHI::ShaderStage::Vertex].size();
        s_Data3D.VertexShader = device.CreateShader(vsDesc);
    }

    if (compiledShaders.find(RHI::ShaderStage::Fragment) != compiledShaders.end())
    {
        RHI::ShaderDesc fsDesc{};
        fsDesc.stage = RHI::ShaderStage::Fragment;
        fsDesc.spirvData = (const uint8_t*)compiledShaders[RHI::ShaderStage::Fragment].data();
        fsDesc.spirvSize = compiledShaders[RHI::ShaderStage::Fragment].size();
        s_Data3D.FragmentShader = device.CreateShader(fsDesc);
    }

    RHI::GraphicsPipelineDesc pipeDesc{};
    pipeDesc.vertexShader = s_Data3D.VertexShader;
    pipeDesc.fragmentShader = s_Data3D.FragmentShader;
    pipeDesc.vertexLayout = {
        {RHI::ShaderDataType::Float3, "a_Position"},
        {RHI::ShaderDataType::Float3, "a_Normal"},
        {RHI::ShaderDataType::Float2, "a_TexCoord"}
    };
    
    pipeDesc.pushConstantSize = sizeof(glm::mat4) * 2 + sizeof(int) * 2; // viewProj + model + entityID + textureSlot
    pipeDesc.blendMode = RHI::BlendMode::Alpha;
    pipeDesc.depthTest = true;
    pipeDesc.depthWrite = true;

    pipeDesc.colorAttachmentCount = 2;
    pipeDesc.colorFormats[0] = RHI::TextureFormat::RGBA8_SRGB;
    pipeDesc.colorFormats[1] = RHI::TextureFormat::R32_SINT;

    s_Data3D.ModelPipeline = device.CreateGraphicsPipeline(pipeDesc);

    // Initialize Grid Pipeline
    std::string gridShaderPath = (FileSystem::Get().GetRootPath() / "assets/shaders/Grid.slang").string();
    auto compiledGridShaders = SlangCompiler::CompileToSPIRV(gridShaderPath);

    if (compiledGridShaders.find(RHI::ShaderStage::Vertex) != compiledGridShaders.end())
    {
        RHI::ShaderDesc vsDesc{};
        vsDesc.stage = RHI::ShaderStage::Vertex;
        vsDesc.spirvData = (const uint8_t*)compiledGridShaders[RHI::ShaderStage::Vertex].data();
        vsDesc.spirvSize = compiledGridShaders[RHI::ShaderStage::Vertex].size();
        s_Data3D.GridVertexShader = device.CreateShader(vsDesc);
    }

    if (compiledGridShaders.find(RHI::ShaderStage::Fragment) != compiledGridShaders.end())
    {
        RHI::ShaderDesc fsDesc{};
        fsDesc.stage = RHI::ShaderStage::Fragment;
        fsDesc.spirvData = (const uint8_t*)compiledGridShaders[RHI::ShaderStage::Fragment].data();
        fsDesc.spirvSize = compiledGridShaders[RHI::ShaderStage::Fragment].size();
        s_Data3D.GridFragmentShader = device.CreateShader(fsDesc);
    }

    RHI::GraphicsPipelineDesc gridPipeDesc{};
    gridPipeDesc.vertexShader = s_Data3D.GridVertexShader;
    gridPipeDesc.fragmentShader = s_Data3D.GridFragmentShader;
    gridPipeDesc.vertexLayout = {}; // Empty vertex layout, using gl_VertexIndex
    
    gridPipeDesc.pushConstantSize = sizeof(glm::mat4) * 2; // viewProj + inverseViewProj
    gridPipeDesc.blendMode = RHI::BlendMode::Alpha;
    gridPipeDesc.depthTest = true;
    gridPipeDesc.depthWrite = true;

    gridPipeDesc.colorAttachmentCount = 2;
    gridPipeDesc.colorFormats[0] = RHI::TextureFormat::RGBA8_SRGB;
    gridPipeDesc.colorFormats[1] = RHI::TextureFormat::R32_SINT;

    s_Data3D.GridPipeline = device.CreateGraphicsPipeline(gridPipeDesc);

    s_Data3D.WhiteTexture = Texture2D::Create(1, 1);
}

void Renderer3D::Shutdown()
{
    auto& device = Renderer::GetDevice();
    device.DestroyGraphicsPipeline(s_Data3D.ModelPipeline);
    device.DestroyShader(s_Data3D.VertexShader);
    device.DestroyShader(s_Data3D.FragmentShader);
    
    device.DestroyGraphicsPipeline(s_Data3D.GridPipeline);
    device.DestroyShader(s_Data3D.GridVertexShader);
    device.DestroyShader(s_Data3D.GridFragmentShader);
    
    s_Data3D.WhiteTexture.reset();
}

void Renderer3D::BeginScene(const EditorCamera& camera)
{
    s_Data3D.ViewProjection = camera.GetViewProjection();
}

void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
{
    s_Data3D.ViewProjection = camera.GetProjection() * glm::inverse(transform);
}

void Renderer3D::EndScene()
{
}

void Renderer3D::DrawGrid()
{
    auto& cmd = Renderer::GetDevice().GetCurrentCommandBuffer();
    
    cmd.BindPipeline(s_Data3D.GridPipeline);

    struct GridPushConstants
    {
        glm::mat4 viewProj;
        glm::mat4 inverseViewProj;
    } pc;
    pc.viewProj = s_Data3D.ViewProjection;
    pc.inverseViewProj = glm::inverse(s_Data3D.ViewProjection);
    
    cmd.PushConstants(RHI::ShaderStage::AllGraphics, &pc, sizeof(GridPushConstants), 0);

    // Draw 6 vertices for the full-screen quad (generated by SV_VertexID)
    cmd.Draw(6, 0);
}

void Renderer3D::SubmitModel(const RD3d::Model& model, const glm::mat4& transform, int entityID)
{
    auto& cmd = Renderer::GetDevice().GetCurrentCommandBuffer();
    
    cmd.BindPipeline(s_Data3D.ModelPipeline);

    struct PushConstants
    {
        glm::mat4 viewProj;
        glm::mat4 model;
        int entityID;
        int textureSlot;
    } pc;
    pc.viewProj = s_Data3D.ViewProjection;
    pc.model = transform;
    pc.entityID = entityID;
    pc.textureSlot = 0;
    
    // We will push constants per primitive now since textureSlot can change.
    // cmd.PushConstants(RHI::ShaderStage::AllGraphics, &pc, sizeof(PushConstants), 0);

    for (const auto& mesh : model.GetMesh())
    {
        for (const auto& prim : mesh.primitive)
        {
            if (!prim.VertexBuffer || !prim.IndexBuffer)
                continue;

            int textureSlot = s_Data3D.WhiteTexture->GetTextureIndex(); // Default to white texture
            if (prim.materialIndex < model.GetMaterials().size())
            {
                auto& material = model.GetMaterials()[prim.materialIndex];
                if (material.AlbedoTexture)
                {
                    textureSlot = material.AlbedoTexture->GetTextureIndex();
                }
            }

            pc.textureSlot = textureSlot;
            cmd.PushConstants(RHI::ShaderStage::AllGraphics, &pc, sizeof(PushConstants), 0);

            cmd.BindVertexBuffer(prim.VertexBuffer);
            cmd.BindIndexBuffer(prim.IndexBuffer);
            cmd.DrawIndexed(prim.IndexCount);
        }
    }
}

} // namespace UHE
