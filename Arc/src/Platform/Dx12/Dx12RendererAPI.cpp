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

	void Dx12RendererAPI::SetClearColor(void* commandList, const glm::vec4& color)
	{
		ARC_PROFILE_SCOPE();
		
		ID3D12GraphicsCommandList9* cmdList = reinterpret_cast<ID3D12GraphicsCommandList9*>(commandList);
		cmdList->ClearRenderTargetView(Dx12Context::GetRtv(), glm::value_ptr(color), 0, nullptr);
	}

	void Dx12RendererAPI::Clear()
	{
	}

	void Dx12RendererAPI::DrawIndexed(void* commandList, const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexArray->Bind(commandList);
		indexCount = indexCount == 0 ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;

		ID3D12GraphicsCommandList9* cmdList = reinterpret_cast<ID3D12GraphicsCommandList9*>(commandList);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}

	void Dx12RendererAPI::Draw(void* commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexBuffer->Bind(commandList);
		ID3D12GraphicsCommandList9* cmdList = reinterpret_cast<ID3D12GraphicsCommandList9*>(commandList);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->DrawInstanced(vertexCount, 1, 0, 0);
	}

	void Dx12RendererAPI::DrawLines(void* commandList, const Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
	{
		ARC_PROFILE_SCOPE();

		vertexBuffer->Bind(commandList);
		ID3D12GraphicsCommandList9* cmdList = reinterpret_cast<ID3D12GraphicsCommandList9*>(commandList);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		cmdList->DrawInstanced(vertexCount, 1, 0, 0);
	}

	void Dx12RendererAPI::ComputeDispatch(void* commandList, uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
	{
		ARC_PROFILE_SCOPE();

		ID3D12GraphicsCommandList9* cmdList = reinterpret_cast<ID3D12GraphicsCommandList9*>(commandList);
		cmdList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	void* Dx12RendererAPI::GetNewGraphicsCommandList()
	{
		return m_Context->GetNewGraphicsCommandList();
	}
	
	void Dx12RendererAPI::Execute(void* commandList)
	{
		ARC_PROFILE_SCOPE();

		m_Context->Execute(reinterpret_cast<ID3D12GraphicsCommandList9*>(commandList));
	}
}
