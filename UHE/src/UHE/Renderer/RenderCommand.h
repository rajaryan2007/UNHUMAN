#pragma once

#include "RendererAPI.h"

namespace UHE {
	class UHE_API RenderCommand
	{
	public:
		inline static void Init() {
			UHE_PROFILE_FUNCTION();
			// Initialize the RendererAPI
			s_RendererAPI->Init();
		}	

		inline static void SetViewport(u32 x, u32 y, u32 width, u32 height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		};

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		};
		inline static void Clear() {
			s_RendererAPI->Clear();
		};

        inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray,u32 indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}
	private:
		static RendererAPI* s_RendererAPI;

	};
}