#include "arcpch.h"
#include "Arc/Renderer/VertexArray.h"

#include "Arc/Renderer/Renderer.h"
#include "Platform/Dx12/Dx12VertexArray.h"

namespace ArcEngine
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::Dx12:	return CreateRef<Dx12VertexArray>();
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
