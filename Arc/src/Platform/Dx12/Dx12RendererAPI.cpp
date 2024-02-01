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

	void Dx12RendererAPI::SetViewport([[maybe_unused]] uint32_t x, [[maybe_unused]] uint32_t y, uint32_t width, uint32_t height)
	{
		m_Context->ResizeSwapchain(width, height);
	}

	void Dx12RendererAPI::SetClearColor(GraphicsCommandList commandList, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE();
		
		D3D12GraphicsCommandList* cmdList = reinterpret_cast<D3D12GraphicsCommandList*>(commandList);
		cmdList->ClearRenderTargetView(Dx12Context::GetRtv(), glm::value_ptr(color), 0, nullptr);
	}

	void Dx12RendererAPI::Clear()
	{
	}

	void Dx12RendererAPI::DrawIndexed(GraphicsCommandList commandList, const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexArray->Bind(commandList);
		indexCount = indexCount == 0 ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;

		D3D12GraphicsCommandList* cmdList = reinterpret_cast<D3D12GraphicsCommandList*>(commandList);
		cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}

	void Dx12RendererAPI::Draw(GraphicsCommandList commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexBuffer->Bind(commandList);
		D3D12GraphicsCommandList* cmdList = reinterpret_cast<D3D12GraphicsCommandList*>(commandList);
		cmdList->DrawInstanced(vertexCount, 1, 0, 0);
	}

	void Dx12RendererAPI::DrawLines(GraphicsCommandList commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexBuffer->Bind(commandList);
		D3D12GraphicsCommandList* cmdList = reinterpret_cast<D3D12GraphicsCommandList*>(commandList);
		cmdList->DrawInstanced(vertexCount, 1, 0, 0);
	}

	void Dx12RendererAPI::ComputeDispatch(GraphicsCommandList commandList, uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
	{
		ARC_PROFILE_SCOPE();

		D3D12GraphicsCommandList* cmdList = reinterpret_cast<D3D12GraphicsCommandList*>(commandList);
		cmdList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	GraphicsCommandList Dx12RendererAPI::BeginRecordingCommandList()
	{
		return m_Context->BeginRecordingCommandList();
	}
	
	void Dx12RendererAPI::EndRecordingCommandList(GraphicsCommandList commandList, bool execute)
	{
		return m_Context->EndRecordingCommandList(commandList, execute);
	}

	void Dx12RendererAPI::ExecuteCommandList(GraphicsCommandList commandList)
	{
		ARC_PROFILE_SCOPE();

		m_Context->ExecuteCommandList(commandList);
	}
}
