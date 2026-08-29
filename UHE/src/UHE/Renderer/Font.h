#pragma once
#include "uhepch.h"
#include "UHE/Core/Core.h"
#include "UHE/RHI/RHITypes.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace UHE {

struct UHE_API FontGlyph
{
    glm::vec2 PlaneBoundsMin, PlaneBoundsMax;
    glm::vec2 UVMin, UVMax;
    f32 Advance = 0.0f;
};

class UHE_API Font2D
{
public:
    Font2D(const std::string& ttfPath, u32 genSizePx = 64, f32 pixelRange = 2.0f);
    ~Font2D();

    Font2D(const Font2D&) = delete;
    Font2D& operator=(const Font2D&) = delete;
    Font2D(Font2D&&) = delete;
    Font2D& operator=(Font2D&&) = delete;

    static Ref<Font2D> Get(const std::string& ttfPath, u32 genSizePx = 64);
    static Ref<Font2D> GetDefault();
    static void Shutdown();

    bool IsValid() const { return m_Valid; }

    const FontGlyph* GetGlyph(u32 codepoint) const;
    f32 GetAscent() const { return m_Ascent; }
    f32 GetLineHeight() const { return m_LineHeight; }
    f32 GetPixelRange() const { return m_PixelRange; }

    RHI::TextureHandle GetAtlas() const { return m_Atlas; }
    u32 GetAtlasWidth() const { return m_AtlasWidth; }
    u32 GetAtlasHeight() const { return m_AtlasHeight; }

private:
    std::string m_Path;
    u32 m_GenSizePx;
    f32 m_PixelRange;
    f32 m_Ascent = 0.0f;
    f32 m_LineHeight = 0.0f;
    u32 m_AtlasWidth = 0;
    u32 m_AtlasHeight = 0;
    bool m_Valid = false;
    RHI::TextureHandle m_Atlas = nullptr;
    std::unordered_map<u32, FontGlyph> m_Glyphs;

    struct Impl;
    std::unique_ptr<Impl> m_Impl;
    static std::unordered_map<std::string, Ref<Font2D>> s_Cache;
    static Ref<Font2D> s_DefaultFont;
};

} // namespace UHE