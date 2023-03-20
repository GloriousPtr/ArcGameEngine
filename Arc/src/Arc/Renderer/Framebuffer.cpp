#include "arcpch.h"
#include "Arc/Renderer/Framebuffer.h"

#include "Arc/Renderer/Renderer.h"
#include "Platform/Dx12/Dx12Framebuffer.h"

namespace ArcEngine
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!") return nullptr;
			case RendererAPI::API::Dx12:	return CreateRef<Dx12Framebuffer>(spec);
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}
}
