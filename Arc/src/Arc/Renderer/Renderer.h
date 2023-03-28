#pragma once

#include "Arc/Renderer/RenderCommand.h"
#include "Arc/Renderer/Shader.h"

namespace ArcEngine
{
	struct GlobalData
	{
		glm::mat4 CameraView{};
		glm::mat4 CameraProjection{};
		glm::mat4 CameraViewProjection{};
		glm::vec4 CameraPosition{};
		uint32_t NumDirectionalLights = 0;
		uint32_t NumPointLights = 0;
		uint32_t NumSpotLights = 0;
	};

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
