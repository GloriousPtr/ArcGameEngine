#include "arcpch.h"
#include "Arc/Renderer/GraphicsContext.h"

#include "Arc/Renderer/Renderer.h"
#include "Platform/Dx12/Dx12Context.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace ArcEngine
{
	Scope<GraphicsContext> GraphicsContext::Create(WindowHandle window)
	{
		switch(Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!") return nullptr;
			case RendererAPI::API::OpenGL:	return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
			case RendererAPI::API::Dx12:	return CreateScope<Dx12Context>(static_cast<HWND>(window));
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}
}
