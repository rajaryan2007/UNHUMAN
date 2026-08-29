#include "uhepch.h"
#include "Font.h"

#include "UHE/Renderer/Renderer.h"
#include "UHE/RHI/RHIDevice.h"
#include "UHE/RHI/RHICommadBuffer.h"
#include "UHE/AssestsManager/VfsSystem.h"

#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "msdf-atlas-gen/FontGeometry.h"
#include "msdf-atlas-gen/GlyphGeometry.h"

#include "msdfgen.h"
#include "msdfgen-ext.h"

#include <algorithm>
#include <thread>

namespace UHE {

struct Font2D::Impl
{
    std::vector<msdf_atlas::GlyphGeometry> Glyphs;
    msdf_atlas::FontGeometry FontGeometry;

    Impl()
        : FontGeometry(&Glyphs)
    {
    }
};

template <typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
static RHI::TextureHandle CreateAtlasTexture(const std::vector<msdf_atlas::GlyphGeometry>& glyphs, uint32_t width,
                                             uint32_t height)
{
    msdf_atlas::GeneratorAttributes attributes;
    attributes.config.overlapSupport = true;
    attributes.scanlinePass = true;

    msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
    generator.setAttributes(attributes);
    unsigned hwThreads = std::thread::hardware_concurrency();
    generator.setThreadCount((int)std::max(1u, std::min(8u, hwThreads ? hwThreads : 1u)));
    generator.generate(glyphs.data(), (int)glyphs.size());

    msdfgen::BitmapConstSection<T, N> bitmap = (msdfgen::BitmapConstSection<T, N>)generator.atlasStorage();

    auto& device = Renderer::GetDevice();
    RHI::TextureDesc texDesc{};
    texDesc.width = width;
    texDesc.height = height;
    texDesc.format = RHI::TextureFormat::RGBA8_UNORM;
    texDesc.usage = RHI::TextureUsage::Sampled | RHI::TextureUsage::TransferDst;
    RHI::TextureHandle texture = device.CreateTexture(texDesc);
    device.GetCurrentCommandBuffer().UpdateTexture(texture, (void*)bitmap.pixels, width * height * N);
    return texture;
}

Font2D::Font2D(const std::string& ttfPath, u32 genSizePx, f32 pixelRange)
    : m_Path(ttfPath), m_GenSizePx(genSizePx), m_PixelRange(pixelRange), m_Impl(std::make_unique<Impl>())
{
    msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
    if (!ft)
    {
        UHE_CORE_ERROR("Failed to initialize FreeType for font: {}", m_Path);
        return;
    }

    msdfgen::FontHandle* font = msdfgen::loadFont(ft, m_Path.c_str());
    if (!font)
    {
        UHE_CORE_ERROR("Failed to load font: {}", m_Path);
        msdfgen::deinitializeFreetype(ft);
        return;
    }

    struct CharsetRange
    {
        uint32_t Begin, End;
    };

    static const CharsetRange charsetRanges[] = {{0x0020, 0x00FF}};

    msdf_atlas::Charset charset;
    for (CharsetRange range : charsetRanges)
    {
        for (uint32_t c = range.Begin; c <= range.End; c++)
            charset.add(c);
    }

    double fontScale = 1.0;
    int glyphsLoaded = m_Impl->FontGeometry.loadCharset(font, fontScale, charset);
    UHE_CORE_INFO("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());
    if (glyphsLoaded <= 0)
    {
        UHE_CORE_ERROR("No glyphs loaded from font: {}", m_Path);
        msdfgen::destroyFont(font);
        msdfgen::deinitializeFreetype(ft);
        return;
    }

    double emSize = (double)m_GenSizePx;

    msdf_atlas::TightAtlasPacker atlasPacker;
    atlasPacker.setPixelRange(m_PixelRange);
    atlasPacker.setMiterLimit(1.0);
    atlasPacker.setScale(emSize);
    int remaining = atlasPacker.pack(m_Impl->Glyphs.data(), (int)m_Impl->Glyphs.size());
    if (remaining != 0)
    {
        UHE_CORE_ERROR("Atlas packing failed for font {}: {} glyphs did not fit", m_Path, remaining);
        msdfgen::destroyFont(font);
        msdfgen::deinitializeFreetype(ft);
        return;
    }

    int width, height;
    atlasPacker.getDimensions(width, height);
    m_AtlasWidth = (u32)width;
    m_AtlasHeight = (u32)height;

    constexpr double DEFAULT_ANGLE_THRESHOLD = 3.0;
    constexpr unsigned long long LCG_MULTIPLIER = 6364136223846793005ull;
    constexpr unsigned long long LCG_INCREMENT = 1442695040888963407ull;
    unsigned long long glyphSeed = 0;
    for (msdf_atlas::GlyphGeometry& glyph : m_Impl->Glyphs)
    {
        glyphSeed = glyphSeed * LCG_MULTIPLIER + LCG_INCREMENT;
        glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
    }

    m_Atlas = CreateAtlasTexture<uint8_t, float, 4, msdf_atlas::mtsdfGenerator>(m_Impl->Glyphs, width, height);
    if (!m_Atlas)
    {
        UHE_CORE_ERROR("Failed to create MSDF atlas for font: {}", m_Path);
        msdfgen::destroyFont(font);
        msdfgen::deinitializeFreetype(ft);
        return;
    }

    const msdfgen::FontMetrics& metrics = m_Impl->FontGeometry.getMetrics();
    m_Ascent = (f32)metrics.ascenderY;
    m_LineHeight = (f32)metrics.lineHeight;

    for (const msdf_atlas::GlyphGeometry& glyph : m_Impl->Glyphs)
    {
        int codepoint = glyph.getIdentifier(msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT);
        if (codepoint < 0)
            continue;

        double planeL, planeB, planeR, planeT;
        glyph.getQuadPlaneBounds(planeL, planeB, planeR, planeT);
        double atlasL, atlasB, atlasR, atlasT;
        glyph.getQuadAtlasBounds(atlasL, atlasB, atlasR, atlasT);

        FontGlyph fontGlyph;
        fontGlyph.PlaneBoundsMin = {(f32)planeL, (f32)planeB};
        fontGlyph.PlaneBoundsMax = {(f32)planeR, (f32)planeT};
        fontGlyph.UVMin = {(f32)atlasL / (f32)m_AtlasWidth, 1.0f - (f32)atlasT / (f32)m_AtlasHeight};
        fontGlyph.UVMax = {(f32)atlasR / (f32)m_AtlasWidth, 1.0f - (f32)atlasB / (f32)m_AtlasHeight};
        fontGlyph.Advance = (f32)glyph.getAdvance();

        m_Glyphs.emplace((u32)codepoint, fontGlyph);
    }

    m_Valid = true;

    UHE_CORE_INFO("Font loaded: {}, {} glyphs, atlas {}x{}", m_Path, (u32)m_Glyphs.size(), m_AtlasWidth,
                  m_AtlasHeight);

    msdfgen::destroyFont(font);
    msdfgen::deinitializeFreetype(ft);
}

Font2D::~Font2D()
{
    if (m_Atlas)
        Renderer::GetDevice().DestroyTexture(m_Atlas);
}

Ref<Font2D> Font2D::Get(const std::string& ttfPath, u32 genSizePx)
{
    std::string key = ttfPath + "#" + std::to_string(genSizePx);
    auto it = s_Cache.find(key);
    if (it != s_Cache.end())
        return it->second;

    Ref<Font2D> font = CreateRef<Font2D>(ttfPath, genSizePx);
    if (!font->IsValid())
    {
        UHE_CORE_ERROR("Failed to create font: {} (not cached)", ttfPath);
        return nullptr;
    }

    s_Cache[key] = font;
    return font;
}

Ref<Font2D> Font2D::GetDefault()
{
    if (!s_DefaultFont)
    {
        s_DefaultFont = Get((FileSystem::Get().GetRootPath() / "assets/fonts/Inter_18pt-Bold.ttf").string());
    }
    return s_DefaultFont;
}

void Font2D::Shutdown()
{
    s_DefaultFont.reset();
    s_Cache.clear();
}

const FontGlyph* Font2D::GetGlyph(u32 codepoint) const
{
    auto it = m_Glyphs.find(codepoint);
    if (it != m_Glyphs.end())
        return &it->second;
    return nullptr;
}

std::unordered_map<std::string, Ref<Font2D>> Font2D::s_Cache;
Ref<Font2D> Font2D::s_DefaultFont;

} // namespace UHE