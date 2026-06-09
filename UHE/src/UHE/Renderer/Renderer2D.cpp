#include "Renderer2D.h"
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "UHE/RHI/RHICommadBuffer.h"
#include "UHE/RHI/RHIDevice.h"
#include "UHE/Renderer/Renderer.h"
#include "UHE/Renderer/SlangCompiler.h"
#include "UHE/Scene/Components.h"

namespace UHE
{

struct QuadVertex
{
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 TexCoord;
    float TexIndex;
    float TilingFactor;
    int EntityID;
};

struct Renderer2DData
{
    static const uint32_t MaxQuads = 20000;
    static const uint32_t MaxVertices = MaxQuads * 4;
    static const uint32_t MaxIndices = MaxQuads * 6;
    static const uint32_t MaxTextureSlots = 32;

    RHI::PipelineHandle QuadPipeline;
    RHI::ShaderHandle QuadVertexShader;
    RHI::ShaderHandle QuadFragmentShader;
    RHI::BufferHandle QuadVertexBuffer;
    RHI::BufferHandle QuadIndexBuffer;

    uint32_t QuadIndexCount = 0;
    QuadVertex* QuadVertexBufferBase = nullptr;
    QuadVertex* QuadVertexBufferPtr = nullptr;

    std::array<RHI::TextureHandle, MaxTextureSlots> TextureSlots;
    uint32_t TextureSlotIndex = 1; // 0 is white texture

    glm::vec4 QuadVertexPositions[4];

    Renderer2D::Statistics Stats;

    RHI::TextureHandle WhiteTexture;
};

static Renderer2DData s_Data;

void Renderer2D::Init()
{
    auto& device = Renderer::GetDevice();

    // Create Vertex Buffer
    RHI::BufferDesc vboDesc{};
    vboDesc.size = s_Data.MaxVertices * sizeof(QuadVertex);
    vboDesc.usage = RHI::BufferUsage::Vertex;
    vboDesc.hostVisible = true;
    vboDesc.debugName = "QuadVertexBuffer";
    s_Data.QuadVertexBuffer = device.CreateBuffer(vboDesc);

    // Create Index Buffer
    uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
    uint32_t offset = 0;
    for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
    {
        quadIndices[i + 0] = offset + 0;
        quadIndices[i + 1] = offset + 1;
        quadIndices[i + 2] = offset + 2;

        quadIndices[i + 3] = offset + 2;
        quadIndices[i + 4] = offset + 3;
        quadIndices[i + 5] = offset + 0;
        offset += 4;
    }

    RHI::BufferDesc iboDesc{};
    iboDesc.size = s_Data.MaxIndices * sizeof(uint32_t);
    iboDesc.usage = RHI::BufferUsage::Index;
    iboDesc.hostVisible = true; // For simple upload, though typically staging buffer is used
    iboDesc.debugName = "QuadIndexBuffer";
    s_Data.QuadIndexBuffer = device.CreateBuffer(iboDesc);

    device.GetCurrentCommandBuffer().UpdateBuffer(s_Data.QuadIndexBuffer, quadIndices, iboDesc.size, 0);

    s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

    uint32_t whiteTextureData = 0xffffffff;
    RHI::TextureDesc whiteTexDesc{};
    whiteTexDesc.width = 1;
    whiteTexDesc.height = 1;
    whiteTexDesc.format = RHI::TextureFormat::RGBA8_UNORM;
    whiteTexDesc.usage = RHI::TextureUsage::Sampled | RHI::TextureUsage::TransferDst;
    s_Data.WhiteTexture = device.CreateTexture(whiteTexDesc);

    device.GetCurrentCommandBuffer().UpdateTexture(s_Data.WhiteTexture, &whiteTextureData, sizeof(uint32_t));

    s_Data.TextureSlots[0] = s_Data.WhiteTexture;

    // Compile Shader
    std::string shaderPath = "../../UHE_EDITOR/assets/shaders/Texture.slang"; // Path relative to runtime
    auto compiledShaders = SlangCompiler::CompileToSPIRV(shaderPath);

    if (compiledShaders.find(RHI::ShaderStage::Vertex) != compiledShaders.end())
    {
        RHI::ShaderDesc vsDesc{};
        vsDesc.stage = RHI::ShaderStage::Vertex;
        vsDesc.spirvData = (const uint8_t*)compiledShaders[RHI::ShaderStage::Vertex].data();
        vsDesc.spirvSize = compiledShaders[RHI::ShaderStage::Vertex].size();
        s_Data.QuadVertexShader = device.CreateShader(vsDesc);
        UHE_CORE_ASSERT(s_Data.QuadVertexShader, "Vertex Shader Creation Failed!");
    }
    else
    {
        UHE_CORE_ERROR("Vertex Shader not compiled!");
    }

    if (compiledShaders.find(RHI::ShaderStage::Fragment) != compiledShaders.end())
    {
        RHI::ShaderDesc fsDesc{};
        fsDesc.stage = RHI::ShaderStage::Fragment;
        fsDesc.spirvData = (const uint8_t*)compiledShaders[RHI::ShaderStage::Fragment].data();
        fsDesc.spirvSize = compiledShaders[RHI::ShaderStage::Fragment].size();
        s_Data.QuadFragmentShader = device.CreateShader(fsDesc);
        UHE_CORE_ASSERT(s_Data.QuadFragmentShader, "Fragment Shader Creation Failed!");
    }
    else
    {
        UHE_CORE_ERROR("Fragment Shader not compiled!");
    }

    RHI::GraphicsPipelineDesc pipeDesc{};
    pipeDesc.vertexShader = s_Data.QuadVertexShader;
    pipeDesc.fragmentShader = s_Data.QuadFragmentShader;
    pipeDesc.vertexLayout = {{RHI::ShaderDataType::Float3, "a_Position"},    {RHI::ShaderDataType::Float4, "a_Color"},
                             {RHI::ShaderDataType::Float2, "a_TexCoord"},    {RHI::ShaderDataType::Float, "a_TexIndex"},
                             {RHI::ShaderDataType::Float, "a_TilingFactor"}, {RHI::ShaderDataType::Int, "a_EntityID"}};
    pipeDesc.pushConstantSize = sizeof(glm::mat4); // viewProjection
    pipeDesc.blendMode = RHI::BlendMode::Alpha;
    pipeDesc.depthTest = true;
    pipeDesc.depthWrite = true;

    // This should match the Framebuffer color format
    pipeDesc.colorAttachmentCount = 2;
    pipeDesc.colorFormats[0] = RHI::TextureFormat::RGBA8_SRGB;
    pipeDesc.colorFormats[1] = RHI::TextureFormat::R32_SINT;

    s_Data.QuadPipeline = device.CreateGraphicsPipeline(pipeDesc);
    UHE_CORE_ASSERT(s_Data.QuadPipeline, "Failed to create QuadPipeline!");

    s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
    s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
    s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
    s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
}

void Renderer2D::Shutdown()
{
    auto& device = Renderer::GetDevice();
    delete[] s_Data.QuadVertexBufferBase;
    device.DestroyGraphicsPipeline(s_Data.QuadPipeline);
    device.DestroyShader(s_Data.QuadVertexShader);
    device.DestroyShader(s_Data.QuadFragmentShader);
    device.DestroyBuffer(s_Data.QuadVertexBuffer);
    device.DestroyBuffer(s_Data.QuadIndexBuffer);
    device.DestroyTexture(s_Data.WhiteTexture);
}

void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
{
    glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

    auto& cmd = Renderer::GetDevice().GetCurrentCommandBuffer();
    cmd.BindPipeline(s_Data.QuadPipeline);
    cmd.PushConstants(RHI::ShaderStage::AllGraphics, &viewProj, sizeof(glm::mat4), 0);

    StartBatch();
}

void Renderer2D::BeginScene(const EditorCamera& camera)
{
    glm::mat4 viewProj = camera.GetViewProjection();

    auto& cmd = Renderer::GetDevice().GetCurrentCommandBuffer();
    cmd.BindPipeline(s_Data.QuadPipeline);
    cmd.PushConstants(RHI::ShaderStage::AllGraphics, &viewProj, sizeof(glm::mat4), 0);

    StartBatch();
}

void Renderer2D::EndScene()
{
    Flush();
}

void Renderer2D::StartBatch()
{
    s_Data.QuadIndexCount = 0;
    s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
    s_Data.TextureSlotIndex = 1;
}

void Renderer2D::Flush()
{
    if (s_Data.QuadIndexCount == 0)
        return; // Nothing to draw

    uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);

    auto& device = Renderer::GetDevice();
    auto& cmd = device.GetCurrentCommandBuffer();

    cmd.UpdateBuffer(s_Data.QuadVertexBuffer, s_Data.QuadVertexBufferBase, dataSize, 0);

    // Bind textures
    for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
    {
        cmd.BindTexture(i, s_Data.TextureSlots[i]);
    }

    cmd.BindVertexBuffer(s_Data.QuadVertexBuffer, 0);
    cmd.BindIndexBuffer(s_Data.QuadIndexBuffer, 0);
    cmd.DrawIndexed(s_Data.QuadIndexCount, 0, 0);

    s_Data.Stats.DrawCalls++;
}

void Renderer2D::NextBatch()
{
    Flush();
    StartBatch();
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
{
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
{
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

    DrawQuad(transform, color);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture,
                          float tilingFactor, const glm::vec4& tintColor)
{
    DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture,
                          float tilingFactor, const glm::vec4& tintColor)
{
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
{
    constexpr size_t quadVertexCount = 4;
    const float textureIndex = 0.0f; // White Texture
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const float tilingFactor = 1.0f;

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        NextBatch();

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
        s_Data.QuadVertexBufferPtr->Color = color;
        s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr->EntityID = entityID;
        s_Data.QuadVertexBufferPtr++;
    }

    s_Data.QuadIndexCount += 6;
    s_Data.Stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor,
                          const glm::vec4& tintColor, int entityID)
{
    constexpr size_t quadVertexCount = 4;
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        NextBatch();

    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
    {
        if (s_Data.TextureSlots[i] == texture->GetTextureHandle())
        {
            textureIndex = (float)i;
            break;
        }
    }

    if (textureIndex == 0.0f)
    {
        if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
            NextBatch();

        textureIndex = (float)s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture->GetTextureHandle();
        s_Data.TextureSlotIndex++;
    }

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr->EntityID = entityID;
        s_Data.QuadVertexBufferPtr++;
    }

    s_Data.QuadIndexCount += 6;
    s_Data.Stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subtexture, float tilingFactor,
                          const glm::vec4& tintColor, int entityID)
{
    constexpr size_t quadVertexCount = 4;
    const glm::vec2* textureCoords = subtexture->GetTexCoords();
    const Ref<Texture2D> texture = subtexture->GetTexture();

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        NextBatch();

    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
    {
        if (s_Data.TextureSlots[i] == texture->GetTextureHandle())
        {
            textureIndex = (float)i;
            break;
        }
    }

    if (textureIndex == 0.0f)
    {
        if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
            NextBatch();

        textureIndex = (float)s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture->GetTextureHandle();
        s_Data.TextureSlotIndex++;
    }

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr->EntityID = entityID;
        s_Data.QuadVertexBufferPtr++;
    }

    s_Data.QuadIndexCount += 6;
    s_Data.Stats.QuadCount++;
}

void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
{
    if (src.Texture)
        DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
    else
        DrawQuad(transform, src.Color, entityID);
}

void Renderer2D::ResetStats()
{
    memset(&s_Data.Stats, 0, sizeof(Statistics));
}

Renderer2D::Statistics Renderer2D::GetStats()
{
    return s_Data.Stats;
}
} // namespace UHE
