#include "arcpch.h"
#include "Arc/Renderer/Renderer.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"

#include "Shader.h"

namespace ArcEngine
{
	Scope<PipelineLibrary> Renderer::s_PipelineLibrary;

	void Renderer::Init()
	{
		ARC_PROFILE_SCOPE();

		RenderCommand::Init();

		s_PipelineLibrary = CreateScope<PipelineLibrary>();
		
		Renderer2D::Init();
		Renderer3D::Init();
	}

	void Renderer::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		s_PipelineLibrary.reset();

		Renderer2D::Shutdown();
		Renderer3D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE();

		RenderCommand::SetViewport(0, 0, width, height);
	}
}
