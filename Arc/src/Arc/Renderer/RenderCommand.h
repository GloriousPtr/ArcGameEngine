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
		
		inline static void SetClearColor(void* commandList, const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(commandList, color);
		}
		
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}
		
		inline static void DrawIndexed(void* commandList, const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(commandList, vertexArray, indexCount);
		}

		inline static void Draw(void* commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t count)
		{
			s_RendererAPI->Draw(commandList, vertexBuffer, count);
		}

		inline static void DrawLines(void* commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(commandList, vertexBuffer, vertexCount);
		}

		inline static void ComputeDispatch(void* commandList, uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
		{
			s_RendererAPI->ComputeDispatch(commandList, threadGroupX, threadGroupY, threadGroupZ);
		}

		inline static void* GetNewGraphicsCommandList()
		{
			return s_RendererAPI->GetNewGraphicsCommandList();
		}

		inline static void Execute(void* commandList)
		{
			s_RendererAPI->Execute(commandList);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}
