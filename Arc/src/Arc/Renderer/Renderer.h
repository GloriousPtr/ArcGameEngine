#pragma once

#include "Arc/Renderer/RenderCommand.h"
#include "Arc/Renderer/Shader.h"

namespace ArcEngine
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();
		
		static void OnWindowResize(uint32_t width, uint32_t height);
		
		[[nodiscard]] inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		[[nodiscard]] inline static PipelineLibrary& GetPipelineLibrary() { return *s_PipelineLibrary; }
	private:
		static Scope<PipelineLibrary> s_PipelineLibrary;
	};
}
