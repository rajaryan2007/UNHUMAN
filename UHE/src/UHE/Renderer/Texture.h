#pragma once
#include "UHE/Core/Core.h"
#include "UHE/RHI/RHITexture.h"
#include <string>

namespace UHE {

class UHE_API Texture {
public:
    virtual ~Texture() = default;

    virtual u32 GetWidth() const = 0;
    virtual u32 GetHeight() const = 0;

    virtual void Bind(u32 slot = 0) const = 0;
    virtual void* GetImGuiTextureID() = 0;
    virtual RHI::TextureHandle GetTextureHandle() const = 0;

    virtual bool operator==(const Texture& other) const = 0;
};

class UHE_API Texture2D : public Texture {
public:
    virtual ~Texture2D() = default;

    static Ref<Texture2D> Create(const std::string& path);
    static Ref<Texture2D> Create(u32 width, u32 height);
};

} // namespace UHE
