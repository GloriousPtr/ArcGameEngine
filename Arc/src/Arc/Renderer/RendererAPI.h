#pragma once

#include "Arc/Renderer/VertexArray.h"

#include <glm/glm.hpp>

namespace ArcEngine
{
	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;

		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void Draw(uint32_t first, uint32_t count) = 0;

		virtual void EnableCulling() = 0;
		virtual void DisableCulling() = 0;
		virtual void FrontCull() = 0;
		virtual void BackCull() = 0;
		virtual void SetDepthMask(bool value) = 0;
		virtual void SetDepthTest(bool value) = 0;

		inline static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	
	private:
		static API s_API;
	};
}
