#include "arcpch.h"
#include "PipelineState.h"

#include "Renderer.h"
#include "Platform/Dx12/Dx12PipelineState.h"

namespace ArcEngine
{
	Ref<PipelineState> PipelineState::Create(const Ref<Shader>& shader, const PipelineSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!") return nullptr;
			case RendererAPI::API::Dx12:	return CreateRef<Dx12PipelineState>(shader, spec);
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!")
			return nullptr;
	}
}
