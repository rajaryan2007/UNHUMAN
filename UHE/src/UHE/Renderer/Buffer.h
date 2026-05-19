#pragma once
#include "UHE/Core/Core.h"
#include "uhepch.h"

namespace UHE {

#include "UHE/RHI/RHITypes.h"

class UHE_API VertexBuffer {
public:
  virtual ~VertexBuffer() {};

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual const RHI::BufferLayout &GetLayout() const = 0;
  virtual void SetLayout(const RHI::BufferLayout &layout) = 0;

  virtual void SetData(const void *data, u32 size) = 0;

  static Ref<VertexBuffer> Create(u32 size);
  static VertexBuffer *Create(float *vertices, u32 size);
};

class UHE_API IndexBuffer {
public:
  virtual ~IndexBuffer() {};

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual u32 GetCount() const = 0;

  static IndexBuffer *Create(u32 *vertiecs, u32 size);
};

}; // namespace UHE
