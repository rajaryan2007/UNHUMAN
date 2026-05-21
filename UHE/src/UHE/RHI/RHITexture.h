#pragma once
#include "RHITypes.h"

namespace UHE::RHI {
  class RHITexture {
  public:
    virtual ~RHITexture() = default;
    virtual const TextureDesc &GetDesc() const = 0;
  private:

  };
}