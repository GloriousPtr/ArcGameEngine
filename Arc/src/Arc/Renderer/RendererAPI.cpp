#include "arcpch.h"
#include "Arc/Renderer/RendererAPI.h"

#include "Platform/Dx12/Dx12RendererAPI.h"

namespace ArcEngine
{
#if defined(ARC_PLATFORM_LINUX)
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::None;
#elif defined(ARC_PLATFORM_WINDOWS)
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Dx12;
#endif

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
			case RendererAPI::API::None:    ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::Dx12:	return CreateScope<Dx12RendererAPI>();
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
