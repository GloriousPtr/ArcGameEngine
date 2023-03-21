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
		static void OnRender();
		
		[[nodiscard]] inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		[[nodiscard]] inline static PipelineLibrary& GetPipelineLibrary() { return *s_PipelineLibrary; }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_SceneData;
		static Scope<PipelineLibrary> s_PipelineLibrary;
	};
}
