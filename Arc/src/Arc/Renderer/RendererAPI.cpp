#include "arcpch.h"
#include "Arc/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace ArcEngine
{
#if defined(ARC_PLATFORM_LINUX)
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;
#elif defined(ARC_PLATFORM_WINDOWS)
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Dx12;
#endif

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
			case RendererAPI::API::None:    ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!") return nullptr;
			case RendererAPI::API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
			case RendererAPI::API::Dx12:	return CreateScope<OpenGLRendererAPI>();
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}
}
