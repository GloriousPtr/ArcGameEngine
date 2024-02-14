#pragma once

#include "Arc/Renderer/GraphicsContext.h"

namespace ArcEngine
{
	class VertexArray;
	class VertexBuffer;

	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;

		enum class API : uint8_t
		{
			None = 0, Dx12
		};
	public:
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(GraphicsCommandList commandList, const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(GraphicsCommandList commandList, const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void Draw(GraphicsCommandList commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount) = 0;
		virtual void DrawLines(GraphicsCommandList commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount) = 0;

		virtual void ComputeDispatch(GraphicsCommandList commandList, uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

		virtual GraphicsCommandList BeginRecordingCommandList() = 0;
		virtual void EndRecordingCommandList(GraphicsCommandList commandList, bool execute = false) = 0;
		virtual void ExecuteCommandList(GraphicsCommandList commandList) = 0;
		virtual void Flush() = 0;

		[[nodiscard]] static API GetAPI() { return s_API; }
		[[nodiscard]] static Scope<RendererAPI> Create();
	
	private:
		static API s_API;
	};
}
