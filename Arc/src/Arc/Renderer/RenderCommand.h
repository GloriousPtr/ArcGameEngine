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

		inline static void BeginFrame()
		{
			s_RendererAPI->BeginFrame();
		}

		inline static void EndFrame()
		{
			s_RendererAPI->EndFrame();
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

		inline static void Draw(const Ref<VertexBuffer>& vertexBuffer, uint32_t count)
		{
			s_RendererAPI->Draw(vertexBuffer, count);
		}

		inline static void DrawLines(const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(vertexBuffer, vertexCount);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}
