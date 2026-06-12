#include "Renderer2D.h"
#include "UHE/AssestsManager/VfsSystem.h"
#include "UHE/Core/Log.h"
#include "UHE/Renderer/Shader.h"
// #include "UHE/Renderer/VertexArray.h"
#include "uhepch.h"
#include "UHE/Renderer/Camera.h"
// #include "UHE/Renderer/RenderCommand.h"
#include "UHE/Renderer/Texture.h"
#include "glm/gtc/matrix_transform.hpp"

namespace UHE
{

struct QuadVertex
{
    glm::vec3 Position;
    glm::vec2 TexCoord;
    glm::vec4 Color;

    f32 TexIndex;
    f32 TilingFactor;

    // editor only
    i32 EntityID{};
};

struct Renderer2DData
{
    static const u32 MaxQuads = 20000;
    static const u32 MaxVertices = MaxQuads * 4;
    static const u32 MaxIndices = MaxQuads * 6;
    static const u32 MaxTextureSlots = 32; // TODO: RenderCaps

    //  Ref<VertexArray> VertexArray;
    // Ref<VertexBuffer> VertexBuffer;
    Ref<Shader> TextureShader;
    Ref<Texture2D> WhiteTexture;

    u32 QuadIndexCount = 0;
    QuadVertex* QuadVertexBufferBase = nullptr;
    QuadVertex* QuadVertexBufferPtr = nullptr;

    std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
    u32 TextureSlotIndex = 1; // 0

    glm::vec4 QuadVertexPositions[4];

    Renderer2D::Statistics stats;
};

static Renderer2DData s_Data;

void Renderer2D::FlushAndReset()
{
    EndScene();

    s_Data.QuadIndexCount = 0;
    s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
    s_Data.TextureSlotIndex = 1;
}

void Renderer2D::Init()
{
    UHE_PROFILE_FUNCTION();

    s_Data.VertexArray = VertexArray::Create();

    // Allocate memory for the vertex buffer
    s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
    s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

    s_Data.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));

    BufferLayout SVlayout = {{ShaderDataType::Float3, "a_Position"},
                             {ShaderDataType::Float2, "a_TexCoord"},
                             {ShaderDataType::Float4, "a_Color"},
                             {ShaderDataType::Float, "a_TexIndex"},

                             {ShaderDataType::Float, "a_TilingFactor"},
                             {ShaderDataType::Int, "a_EntityID"}

    };

    s_Data.VertexBuffer->SetLayout(SVlayout);
    s_Data.VertexArray->AddVertexBuffer(s_Data.VertexBuffer);

    u32* quadIndices = new u32[s_Data.MaxIndices];

    for (u32 i = 0, offset = 0; i < s_Data.MaxIndices; i += 6, offset += 4)
    {
        quadIndices[i + 0] = offset + 0;
        quadIndices[i + 1] = offset + 1;
        quadIndices[i + 2] = offset + 2;
        quadIndices[i + 3] = offset + 2;
        quadIndices[i + 4] = offset + 3;
        quadIndices[i + 5] = offset + 0;
    }

    std::shared_ptr<UHE::IndexBuffer> squareIB;
    squareIB.reset(UHE::IndexBuffer::Create(quadIndices, s_Data.MaxIndices));

    s_Data.VertexArray->SetIndexBuffer(squareIB);
    delete[] quadIndices;
    u32 whitePixel = 0xffffffff;

    s_Data.WhiteTexture = Texture2D::Create(1, 1);
    s_Data.WhiteTexture->SetData(&whitePixel, sizeof(u32));

    i32 samplers[s_Data.MaxTextureSlots];
    for (u32 i = 0; i < s_Data.MaxTextureSlots; i++)
    {
        samplers[i] = i;
    }

    // s_Data.TextureShader = UHE::Shader::Create(UHE::FileSystem::Get().Resolve("shaders/flatSquare.glsl"));
    s_Data.TextureShader = UHE::Shader::Create(UHE::FileSystem::Get().Resolve("shaders/Texture.slang"));

    s_Data.TextureShader->Bind();
    // s_Data.TextureShader->SetInt("u_Texture", 0);
    // s_Data.TextureShader->SetIntArray("u_Texture", samplers,
    //                                   s_Data.MaxTextureSlots);

    s_Data.TextureSlots[0] = s_Data.WhiteTexture;

    s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
    s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
    s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
    s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
}

void Renderer2D::Shutdown()
{
    UHE_PROFILE_FUNCTION();

    delete[] s_Data.QuadVertexBufferBase;
    s_Data.QuadVertexBufferBase = nullptr;
    s_Data.QuadVertexBufferPtr = nullptr;
}

void Renderer2D::BeginScene(const OrthographicCamera& camera)
{
    UHE_PROFILE_FUNCTION();

    s_Data.TextureShader->Bind();
    // s_Data.TextureShader->SetMat4("u_ViewProjection",
    //                               camera.GetViewProjectionMatrix());
    // s_Data.TextureShader->SetMat4("u_Transform", glm::mat4(1.0f));
    // s_Data.TextureShader->SetFloat4("u_Color", glm::vec4(1.0f));
    // s_Data.TextureShader->SetFloat("v_TilingFactor", 1.0f);
    s_Data.WhiteTexture->Bind();

    s_Data.QuadIndexCount = 0;
    s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

    s_Data.TextureSlotIndex = 1;
}

void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
{

    glm::mat4 viewproj = camera.GetProjection() * glm::inverse(transform);

    s_Data.TextureShader->Bind();
    // s_Data.TextureShader->SetMat4("u_ViewProjection", viewproj);
    // s_Data.TextureShader->SetMat4("u_Transform", glm::mat4(1.0f));
    // s_Data.TextureShader->SetFloat4("u_Color", glm::vec4(1.0f));
    // s_Data.TextureShader->SetFloat("v_TilingFactor", 1.0f);
    s_Data.WhiteTexture->Bind();

    s_Data.QuadIndexCount = 0;
    s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

    s_Data.TextureSlotIndex = 1;
}

void Renderer2D::BeginScene(const EditorCamera& camera)
{
    UHE_PROFILE_FUNCTION();

    glm::mat4 viewproj = camera.GetViewProjection();

    s_Data.TextureShader->Bind();
    // s_Data.TextureShader->SetMat4("u_ViewProjection", viewproj);
    // s_Data.TextureShader->SetMat4("u_Transform", glm::mat4(1.0f));
    // s_Data.TextureShader->SetFloat4("u_Color", glm::vec4(1.0f));
    // s_Data.TextureShader->SetFloat("v_TilingFactor", 1.0f);
    s_Data.WhiteTexture->Bind();

    s_Data.QuadIndexCount = 0;
    s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

    s_Data.TextureSlotIndex = 1;
}

void Renderer2D::flush()
{
    for (u32 i = 0; i < s_Data.TextureSlotIndex; i++)
    {
        s_Data.TextureSlots[i]->Bind(i);
    }

    RenderCommand::DrawIndexed(s_Data.VertexArray, s_Data.QuadIndexCount);

    // s_Data.stats.QuadCount++;
    s_Data.stats.DrawCalls++;
}

void Renderer2D::EndScene()
{
    UHE_PROFILE_FUNCTION();

    if (s_Data.QuadIndexCount == 0)
        return;

    u32 dataSize = (u32)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
    s_Data.VertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
    flush();
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
{
    UHE_PROFILE_FUNCTION();

    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
{
    UHE_PROFILE_FUNCTION();

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
    {
        FlushAndReset();
    }

    const f32 texIndex = 0.0f;

    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
    s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexIndex = texIndex; // White Texture
    s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
    s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
    s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
    s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;

    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture,
                          f32 tiling_factor, const glm::vec4& tintColor)
{

    UHE_PROFILE_FUNCTION();

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
    {
        FlushAndReset();
    }

    if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
    {
        FlushAndReset();
    }

    constexpr glm::vec4 defaultColor = {1.0f, 1.0f, 1.0f, 1.0f};

    f32 textureIndex = 0.0f;
    for (u32 i = 1; i < s_Data.TextureSlots.size(); ++i)
    {
        if (s_Data.TextureSlots[i] && s_Data.TextureSlots[i] == texture)
        {
            textureIndex = (f32)i;
            break;
        }
    }

    if (textureIndex == 0)
    {
        textureIndex = s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
        ++s_Data.TextureSlotIndex;
    };

    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
    s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
    s_Data.QuadVertexBufferPtr->Color = defaultColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
    s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
    s_Data.QuadVertexBufferPtr->Color = defaultColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
    s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
    s_Data.QuadVertexBufferPtr->Color = defaultColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
    s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
    s_Data.QuadVertexBufferPtr->Color = defaultColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;

    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture,
                          f32 tiling_factor, const glm::vec4& tintColor)
{
    UHE_PROFILE_FUNCTION();
    DrawQuad({position.x, position.y, 0}, size, texture, tiling_factor, tintColor);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& Subtexture,
                          f32 tiling_factor /*= 1.0f*/, const glm::vec4& tintColor /*= glm::vec4(1.0f)*/)
{
    DrawQuad({position.x, position.y, 0}, size, Subtexture, tiling_factor, tintColor);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& Subtexture,
                          f32 tiling_factor /*= 1.0f*/, const glm::vec4& tintColor /*= glm::vec4(1.0f)*/)
{
    UHE_PROFILE_FUNCTION();

    constexpr size_t quadVertexCount = 4;
    constexpr glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    const glm::vec2* textureCoords = Subtexture->GetTexCoords();
    const Ref<Texture2D> texture = Subtexture->GetTexture();

    //{ {0.0f,0.0f},{1.0f,0.0f},{1.0f,1.0f },{0.0f,1.0f} };

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
    {
        FlushAndReset();
    }

    if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
    {
        FlushAndReset();
    }

    constexpr glm::vec4 defaultColor = {1.0f, 1.0f, 1.0f, 1.0f};

    f32 textureIndex = 0.0f;
    for (u32 i = 1; i < s_Data.TextureSlots.size(); ++i)
    {
        if (s_Data.TextureSlots[i] && s_Data.TextureSlots[i] == texture)
        {
            textureIndex = (f32)i;
            break;
        }
    }

    if (textureIndex == 0)
    {

        textureIndex = s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
        ++s_Data.TextureSlotIndex;
    };

    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
        s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
        s_Data.QuadVertexBufferPtr->EntityID = -1;
        s_Data.QuadVertexBufferPtr++;
    }
    s_Data.QuadIndexCount += 6;

    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2* texCoords,
                          f32 tiling_factor, const glm::vec4& tintColor)
{
    UHE_PROFILE_FUNCTION();

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        FlushAndReset();

    if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
        FlushAndReset();

    f32 textureIndex = 0.0f;
    for (u32 i = 1; i < s_Data.TextureSlotIndex; i++)
    {
        if (s_Data.TextureSlots[i] && *s_Data.TextureSlots[i] == *texture)
        {
            textureIndex = (f32)i;
            break;
        }
    }

    if (textureIndex == 0.0f)
    {
        textureIndex = (f32)s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
        ++s_Data.TextureSlotIndex;
    }

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
    s_Data.QuadVertexBufferPtr->TexCoord = texCoords[0];
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
    s_Data.QuadVertexBufferPtr->TexCoord = texCoords[1];
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
    s_Data.QuadVertexBufferPtr->TexCoord = texCoords[2];
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
    s_Data.QuadVertexBufferPtr->TexCoord = texCoords[3];
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;
    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& texture, f32 tiling_factor /*= 1.0f*/,
                          const glm::vec4& tintColor /*= glm::vec4(1.0f)*/)
{
    UHE_PROFILE_FUNCTION();

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
    {
        FlushAndReset();
    }

    if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
    {
        FlushAndReset();
    }

    constexpr glm::vec4 defaultColor = {1.0f, 1.0f, 1.0f, 1.0f};

    f32 textureIndex = 0.0f;
    Ref<Texture2D> tex = texture->GetTexture();

    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
    {
        if (s_Data.TextureSlots[i] == tex)
        {
            textureIndex = (float)i;
            break;
        }
    }

    if (textureIndex == 0)
    {
        textureIndex = s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture->GetTexture();
        ++s_Data.TextureSlotIndex;
    };

    const glm::vec2* texCoords = texture->GetTexCoords();

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
    s_Data.QuadVertexBufferPtr->TexCoord = texCoords[0];
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
    s_Data.QuadVertexBufferPtr->TexCoord = texCoords[1];
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
    s_Data.QuadVertexBufferPtr->TexCoord = texCoords[2];
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
    s_Data.QuadVertexBufferPtr->TexCoord = texCoords[3];
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;

    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, f32 tiling_factor,
                          const glm::vec4& color, i32 entity)
{
    UHE_PROFILE_FUNCTION();
    constexpr size_t quadVertexCount = 4;
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        FlushAndReset();

    f32 textureIndex = 0.0f;
    for (u32 i = 1; i < s_Data.TextureSlotIndex; i++)
    {
        if (s_Data.TextureSlots[i] && s_Data.TextureSlots[i] == texture)
        {
            textureIndex = (f32)i;
            break;
        }
    }
    if (textureIndex == 0.0f)
    {
        if (s_Data.TextureSlotIndex >= s_Data.MaxTextureSlots)
            FlushAndReset();
        textureIndex = (f32)s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
        s_Data.TextureSlotIndex++;
    }

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
        s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        s_Data.QuadVertexBufferPtr->Color = color;
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
        s_Data.QuadVertexBufferPtr->EntityID = entity;
        s_Data.QuadVertexBufferPtr++;
    }
    s_Data.QuadIndexCount += 6;
    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity)
{
    UHE_PROFILE_FUNCTION();
    constexpr size_t quadVertexCount = 4;
    const f32 textureIndex = 0.0f; // white texture
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const f32 tilingFactor = 1.0f;

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        FlushAndReset();

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
        s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        s_Data.QuadVertexBufferPtr->Color = color;
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr->EntityID = entity;
        s_Data.QuadVertexBufferPtr++;
    }
    s_Data.QuadIndexCount += 6;
    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, i32 entity)
{
    if (src.Texture)
        DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entity);
    else
        DrawQuad(transform, src.Color, entity);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, f32 rotation, const glm::vec4& color)
{
    UHE_PROFILE_FUNCTION();
    DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color)
{
    UHE_PROFILE_FUNCTION();
    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
    {
        FlushAndReset();
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    const float texIndex = 0.0f; // white texture
    const float tilingFactor = 1.0f;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
    s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
    s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
    s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
    s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
    s_Data.QuadVertexBufferPtr->Color = color;
    s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;
    /*	s_Data.TextureShader->Setf324("u_Color", color);
            s_Data.WhiteTexture->Bind();

            glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
       glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f }) *
       glm::rotate(glm::mat4(1.0f),rotation, { 0.0f,0.0f,1.0f });
            s_Data.TextureShader->SetMat4("u_Transform", transform);

            s_Data.VertexArray->Bind();
            RenderCommand::DrawIndexed(s_Data.VertexArray);*/
    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, f32 rotation,
                                 Ref<Texture2D>& texture, f32 tiling_factor, const glm::vec4& tintColor)
{
    UHE_PROFILE_FUNCTION();

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
    {
        FlushAndReset();
    }

    if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
    {
        FlushAndReset();
    }

    constexpr glm::vec4 defaultColor = {1.0f, 1.0f, 1.0f, 1.0f};

    f32 textureIndex = 0.0f;
    for (u32 i = 1; i < s_Data.TextureSlots.size(); ++i)
    {
        if (s_Data.TextureSlots[i] && s_Data.TextureSlots[i] == texture)
        {
            textureIndex = (f32)i;
            break;
        }
    }

    if (textureIndex == 0)
    {

        textureIndex = s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
        ++s_Data.TextureSlotIndex;
    };

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
    s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
    s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
    s_Data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
    s_Data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
    s_Data.QuadVertexBufferPtr->Color = tintColor;
    s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
    s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
    s_Data.QuadVertexBufferPtr->EntityID = -1;
    s_Data.QuadVertexBufferPtr++;

    s_Data.QuadIndexCount += 6;

    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, f32 rotation,
                                 Ref<Texture2D>& texture, f32 tiling_factor, const glm::vec4& tintColor)
{
    UHE_PROFILE_FUNCTION();
    DrawRotatedQuad({position.x, position.y, 0}, size, rotation, texture, tiling_factor, tintColor);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, f32 rotation,
                                 Ref<SubTexture2D>& Subtexture, f32 tiling_factor /*= 1.0f*/,
                                 const glm::vec4& tintColor /*= glm::vec4(1.0f)*/)
{
    constexpr size_t quadVertexCount = 4;
    constexpr glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    const glm::vec2* textureCoords = Subtexture->GetTexCoords();
    const Ref<Texture2D> texture = Subtexture->GetTexture();

    //{ {0.0f,0.0f},{1.0f,0.0f},{1.0f,1.0f },{0.0f,1.0f} };

    if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
    {
        FlushAndReset();
    }

    if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
    {
        FlushAndReset();
    }

    constexpr glm::vec4 defaultColor = {1.0f, 1.0f, 1.0f, 1.0f};

    f32 textureIndex = 0.0f;
    for (u32 i = 1; i < s_Data.TextureSlots.size(); ++i)
    {
        if (s_Data.TextureSlots[i] && s_Data.TextureSlots[i] == texture)
        {
            textureIndex = (f32)i;
            break;
        }
    }

    if (textureIndex == 0)
    {

        textureIndex = s_Data.TextureSlotIndex;
        s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
        ++s_Data.TextureSlotIndex;
    };

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
        s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tiling_factor;
        s_Data.QuadVertexBufferPtr->EntityID = -1;
        s_Data.QuadVertexBufferPtr++;
    }
    s_Data.QuadIndexCount += 6;

    s_Data.stats.QuadCount++;
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, f32 rotation,
                                 Ref<SubTexture2D>& Subtexture, f32 tiling_factor /*= 1.0f*/,
                                 const glm::vec4& tintColor /*= glm::vec4(1.0f)*/)
{
    DrawRotatedQuad({position.x, position.y, 0}, size, rotation, Subtexture, tiling_factor, tintColor);
}

Renderer2D::Statistics Renderer2D::GetStats()
{
    return s_Data.stats;
}

void Renderer2D::ResetStats()
{
    memset(&s_Data.stats, 0, sizeof(Statistics));
}

} // namespace UHE
