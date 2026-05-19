#pragma once
#include <memory.h>
#include "UHE/Renderer/RendererAPI.h"
#include "UHE/Renderer/VertexArray.h"

namespace UHE {
	class VulkanRendererAPI : RendererAPI {
       virtual void Init() override;
       virtual void SetViewport(u32 x, u32 y, u32 width,
                                   u32 height) override;
       virtual void SetClearColor(const glm::vec4 &color) override;
       virtual void Clear() override;

       virtual void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray,
                                u32 indexCount = 0) override;
    };
}
