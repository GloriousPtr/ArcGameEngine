#include "arcpch.h"
#include "Dx12Buffer.h"

#include <d3dx12/d3dx12.h>

#include "DxHelper.h"
#include "Dx12Allocator.h"

namespace ArcEngine
{
	//////////////////////////////////////////////////////////////////////
	// VertexBuffer //////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	static void CreateBuffer(D3D12MA::Allocation** allocation, uint32_t size, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState)
	{
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		Dx12Allocator::CreateBufferResource(heapType, &resourceDesc, initialState, allocation);
	}

	Dx12VertexBuffer::Dx12VertexBuffer(uint32_t size, uint32_t stride)
	{
		ARC_PROFILE_SCOPE();

		CreateBuffer(&m_Allocation, size, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
		m_BufferView.BufferLocation = m_Allocation->GetResource()->GetGPUVirtualAddress();
		m_BufferView.StrideInBytes = stride;
		m_BufferView.SizeInBytes = size;

		CreateBuffer(&m_UploadAllocation, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

		NameResource(m_Allocation, "VertexBufferAllocation");
		NameResource(m_UploadAllocation, "VertexBufferUpload");
	}

	Dx12VertexBuffer::Dx12VertexBuffer(const float* vertices, uint32_t size, uint32_t stride)
	{
		ARC_PROFILE_SCOPE();

		CreateBuffer(&m_Allocation, size, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
		m_BufferView.BufferLocation = m_Allocation->GetResource()->GetGPUVirtualAddress();
		m_BufferView.StrideInBytes = stride;
		m_BufferView.SizeInBytes = size;

		CreateBuffer(&m_UploadAllocation, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

		NameResource(m_Allocation, "VertexBufferAllocation");
		NameResource(m_UploadAllocation, "VertexBufferUpload");

		Dx12Utils::SetBufferData(m_UploadAllocation, vertices, size);
		GraphicsCommandList commandList = Dx12Context::BeginRecordingCommandList();
		reinterpret_cast<D3D12GraphicsCommandList*>(commandList)->CopyBufferRegion(m_Allocation->GetResource(), 0, m_UploadAllocation->GetResource(), 0, size);
		Dx12Context::EndRecordingCommandList(commandList);
	}

	Dx12VertexBuffer::~Dx12VertexBuffer()
	{
		ARC_PROFILE_SCOPE();

		if (m_UploadAllocation)
			Dx12Context::DeferredRelease(m_UploadAllocation);
		if (m_Allocation)
			Dx12Context::DeferredRelease(m_Allocation);
	}

	void Dx12VertexBuffer::Bind(GraphicsCommandList commandList) const
	{
		ARC_PROFILE_SCOPE();

		reinterpret_cast<D3D12GraphicsCommandList*>(commandList)->IASetVertexBuffers(0, 1, &m_BufferView);
	}

	void Dx12VertexBuffer::SetData(GraphicsCommandList commandList, const void* data, uint32_t size)
	{
		ARC_PROFILE_SCOPE();

		ID3D12Resource* resource = m_Allocation->GetResource();
		ID3D12Resource* uploadResource = m_UploadAllocation->GetResource();
		D3D12GraphicsCommandList* cmdList = reinterpret_cast<D3D12GraphicsCommandList*>(commandList);

		const auto toCommonBarrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->ResourceBarrier(1, &toCommonBarrier);

		Dx12Utils::SetBufferData(m_UploadAllocation, data, size);
		cmdList->CopyBufferRegion(resource, 0, uploadResource, 0, size);

		const auto toVertexBarrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		cmdList->ResourceBarrier(1, &toVertexBarrier);
	}


	//////////////////////////////////////////////////////////////////////
	// IndexBuffer ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	Dx12IndexBuffer::Dx12IndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		ARC_PROFILE_SCOPE();

		const uint32_t size = count * sizeof(uint32_t);

		CreateBuffer(&m_Allocation, size, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
		m_BufferView.BufferLocation = m_Allocation->GetResource()->GetGPUVirtualAddress();
		m_BufferView.Format = DXGI_FORMAT_R32_UINT;
		m_BufferView.SizeInBytes = size;

		CreateBuffer(&m_UploadAllocation, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
		Dx12Utils::SetBufferData(m_UploadAllocation, indices, size);

		NameResource(m_Allocation, "IndexBufferAllocation");
		NameResource(m_UploadAllocation, "IndexBufferUpload");

		GraphicsCommandList commandList = Dx12Context::BeginRecordingCommandList();
		reinterpret_cast<D3D12GraphicsCommandList*>(commandList)->CopyBufferRegion(m_Allocation->GetResource(), 0, m_UploadAllocation->GetResource(), 0, size);
		Dx12Context::EndRecordingCommandList(commandList);
	}

	Dx12IndexBuffer::~Dx12IndexBuffer()
	{
		ARC_PROFILE_SCOPE();

		if (m_UploadAllocation)
			Dx12Context::DeferredRelease(m_UploadAllocation);
		if (m_Allocation)
			Dx12Context::DeferredRelease(m_Allocation);
	}

	void Dx12IndexBuffer::Bind(GraphicsCommandList commandList) const
	{
		ARC_PROFILE_SCOPE();

		reinterpret_cast<D3D12GraphicsCommandList*>(commandList)->IASetIndexBuffer(&m_BufferView);
	}
}
