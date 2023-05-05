#include "arcpch.h"
#include "Dx12RendererAPI.h"

#include <d3d12.h>

#include "Arc/Core/Application.h"
#include "Arc/Core/Window.h"
#include "Arc/Renderer/VertexArray.h"
#include "Dx12Context.h"
#include "Dx12Resources.h"

namespace ArcEngine
{
	void Dx12RendererAPI::Init()
	{
		ARC_PROFILE_SCOPE();

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

	void Dx12RendererAPI::SetViewport([[maybe_unused]] uint32_t x, [[maybe_unused]] uint32_t y, uint32_t width, uint32_t height)
	{
		m_Context->ResizeSwapchain(width, height);
	}

	void Dx12RendererAPI::SetClearColor(const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE();

		Dx12Context::GetGraphicsCommandList()->ClearRenderTargetView(Dx12Context::GetRtv(), glm::value_ptr(color), 0, nullptr);
	}

	void Dx12RendererAPI::Clear()
	{
	}

	void Dx12RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexArray->Bind();
		indexCount = indexCount == 0 ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;

		auto* commandList = Dx12Context::GetGraphicsCommandList();
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}

	void Dx12RendererAPI::Draw(const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexBuffer->Bind();
		auto* commandList = Dx12Context::GetGraphicsCommandList();
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->DrawInstanced(vertexCount, 1, 0, 0);
	}

	void Dx12RendererAPI::DrawLines(const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexBuffer->Bind();
		auto* commandList = Dx12Context::GetGraphicsCommandList();
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		commandList->DrawInstanced(vertexCount, 1, 0, 0);
	}

	void Dx12RendererAPI::ComputeDispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
	{
		Dx12Context::GetGraphicsCommandList()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	void Dx12RendererAPI::Execute()
	{
		auto* commandList = Dx12Context::GetGraphicsCommandList();

		commandList->Close();
		ID3D12CommandList* commandLists[] = { Dx12Context::GetGraphicsCommandList() };
		Dx12Context::GetCommandQueue()->ExecuteCommandLists(1, commandLists);

		Dx12Context::WaitForGpu();
		Dx12Context::GetGraphicsCommandAllocator()->Reset();
		commandList->Reset(Dx12Context::GetGraphicsCommandAllocator(), nullptr);

		ID3D12DescriptorHeap* descriptorHeap = Dx12Context::GetSrvHeap()->Heap();
		commandList->SetDescriptorHeaps(1, &descriptorHeap);
	}
}
