#pragma once

namespace ArcEngine
{
	class VertexArray;

	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;

		enum class API
		{
			None = 0, OpenGL, Dx12
		};
	public:
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void Draw(const Ref<VertexArray>& vertexArray, uint32_t count) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		virtual void EnableCulling() = 0;
		virtual void DisableCulling() = 0;
		virtual void FrontCull() = 0;
		virtual void BackCull() = 0;
		virtual void SetDepthMask(bool value) = 0;
		virtual void SetDepthTest(bool value) = 0;
		virtual void SetBlendState(bool value) = 0;

		[[nodiscard]] static API GetAPI() { return s_API; }
		[[nodiscard]] static Scope<RendererAPI> Create();
	
	private:
		static API s_API;
	};
}
