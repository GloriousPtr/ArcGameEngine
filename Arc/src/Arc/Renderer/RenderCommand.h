#pragma once
#include "Arc/Renderer/RendererAPI.h"

namespace ArcEngine
{
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}
		
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}
		
		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}
		
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}
		
		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		inline static void Draw(uint32_t first, uint32_t count)
		{
			s_RendererAPI->Draw(first, count);
		}

		inline static void FrontCull()
		{
			s_RendererAPI->FrontCull();
		}

		inline static void BackCull()
		{
			s_RendererAPI->BackCull();
		}

		inline static void SetDepthMask(bool value)
		{
			s_RendererAPI->SetDepthMask(value);
		}

		inline static void SetDepthTest(bool value)
		{
			s_RendererAPI->SetDepthTest(value);
		}
	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}
