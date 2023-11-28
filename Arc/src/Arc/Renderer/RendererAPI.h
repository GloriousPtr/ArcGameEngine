#pragma once

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
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void Draw(const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount) = 0;
		virtual void DrawLines(const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount) = 0;

		virtual void ComputeDispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

		virtual void Execute() = 0;

		[[nodiscard]] static API GetAPI() { return s_API; }
		[[nodiscard]] static Scope<RendererAPI> Create();
	
	private:
		static API s_API;
	};
}
