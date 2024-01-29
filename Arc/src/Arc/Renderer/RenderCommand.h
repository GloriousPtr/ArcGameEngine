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
		
		inline static void SetClearColor(GraphicsCommandList commandList, const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(commandList, color);
		}
		
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}
		
		inline static void DrawIndexed(GraphicsCommandList commandList, const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(commandList, vertexArray, indexCount);
		}

		inline static void Draw(GraphicsCommandList commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t count)
		{
			s_RendererAPI->Draw(commandList, vertexBuffer, count);
		}

		inline static void DrawLines(GraphicsCommandList commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(commandList, vertexBuffer, vertexCount);
		}

		inline static void ComputeDispatch(GraphicsCommandList commandList, uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
		{
			s_RendererAPI->ComputeDispatch(commandList, threadGroupX, threadGroupY, threadGroupZ);
		}

		inline static GraphicsCommandList BeginRecordingCommandList()
		{
			return s_RendererAPI->BeginRecordingCommandList();
		}

		inline static void EndRecordingCommandList(GraphicsCommandList commandList, bool execute = false)
		{
			return s_RendererAPI->EndRecordingCommandList(commandList, execute);
		}

		inline static void ExecuteCommandList(GraphicsCommandList commandList)
		{
			s_RendererAPI->ExecuteCommandList(commandList);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}
