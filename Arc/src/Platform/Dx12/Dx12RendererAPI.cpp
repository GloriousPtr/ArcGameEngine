#include "arcpch.h"
#include "Dx12RendererAPI.h"

#include <d3d12.h>

#include "Arc/Core/Application.h"
#include "Arc/Core/Window.h"
#include "Dx12Context.h"

namespace ArcEngine
{
	void Dx12RendererAPI::Init()
	{
		m_Context = reinterpret_cast<Dx12Context*>(Application::Get().GetWindow().GetGraphicsContext().get());
	}

	void Dx12RendererAPI::BeginFrame()
	{
		m_Context->OnBeginFrame();
	}

	void Dx12RendererAPI::EndFrame()
	{
		m_Context->OnEndFrame();
	}

	void Dx12RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		m_Context->ResizeSwapchain(width, height);
	}

	void Dx12RendererAPI::SetClearColor(const glm::vec4& color)
	{
	}

	void Dx12RendererAPI::Clear()
	{
	}

	void Dx12RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
	}

	void Dx12RendererAPI::Draw(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
	}

	void Dx12RendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
	}

	void Dx12RendererAPI::EnableCulling()
	{
	}

	void Dx12RendererAPI::DisableCulling()
	{
	}

	void Dx12RendererAPI::FrontCull()
	{
	}

	void Dx12RendererAPI::BackCull()
	{
	}

	void Dx12RendererAPI::SetDepthMask(bool value)
	{
	}

	void Dx12RendererAPI::SetDepthTest(bool value)
	{
	}

	void Dx12RendererAPI::SetBlendState(bool value)
	{
	}
}
