#include "arcpch.h"
#include "Dx12RendererAPI.h"

#include <d3d12.h>
#include <glm/gtc/type_ptr.inl>

#include "Arc/Core/Application.h"
#include "Arc/Core/Window.h"
#include "Dx12Context.h"
#include "Arc/Renderer/VertexArray.h"

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
		Dx12Context::GetGraphicsCommandList()->ClearRenderTargetView(Dx12Context::GetRtv(), glm::value_ptr(color), 0, nullptr);
	}

	void Dx12RendererAPI::Clear()
	{
	}

	void Dx12RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		indexCount = indexCount == 0 ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;

		auto* commandList = Dx12Context::GetGraphicsCommandList();
		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
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
