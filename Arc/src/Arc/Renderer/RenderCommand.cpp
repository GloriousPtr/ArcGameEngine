#include "arcpch.h"
#include "Arc/Renderer/RenderCommand.h"

namespace ArcEngine
{
	[[nodiscard]] Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}
