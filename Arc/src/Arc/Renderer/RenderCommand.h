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

		inline static void Draw(const Ref<VertexArray>& vertexArray, uint32_t count)
		{
			s_RendererAPI->Draw(vertexArray, count);
		}

		inline static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}

		inline static void EnableCulling()
		{
			s_RendererAPI->EnableCulling();
		}

		inline static void DisableCulling()
		{
			s_RendererAPI->DisableCulling();
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

		inline static void SetBlendState(bool value)
		{
			s_RendererAPI->SetBlendState(value);
		}
	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}
