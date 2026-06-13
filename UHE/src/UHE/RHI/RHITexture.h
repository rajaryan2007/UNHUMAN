#pragma once
#include "RHITypes.h"

namespace UHE::RHI {
  class RHITexture {
  public:
    virtual ~RHITexture() = default;
    virtual const TextureDesc &GetDesc() const = 0;
    virtual void* GetImGuiTextureID() = 0;
    virtual u32 GetTextureIndex() const { return 0; }
  private:

  };
}