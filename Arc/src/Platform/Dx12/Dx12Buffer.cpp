#include "arcpch.h"
#include "Dx12Buffer.h"

#include "d3dx12.h"

namespace ArcEngine
{
	//////////////////////////////////////////////////////////////////////
	// VertexBuffer //////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	static void CreateBuffer(ID3D12Resource** resource, uint32_t size, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState)
	{
		const D3D12_HEAP_PROPERTIES defaultHeap
		{
			heapType,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			1,
			1
		};

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		Dx12Context::GetDevice()->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &desc, initialState, nullptr, IID_PPV_ARGS(resource));
	}

	static void SetBufferData(ID3D12Resource* resource, const void* data, uint32_t size)
	{
		void* vertexDataBegin;
		D3D12_RANGE readRange;
		readRange.Begin = 0;
		readRange.End = 0;

		resource->Map(0, &readRange, &vertexDataBegin);
		memcpy(vertexDataBegin, data, size);
		resource->Unmap(0, nullptr);
	}

	Dx12VertexBuffer::Dx12VertexBuffer(uint32_t size, uint32_t stride)
	{
		CreateBuffer(&m_Resource, size, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
		m_BufferView.BufferLocation = m_Resource->GetGPUVirtualAddress();
		m_BufferView.StrideInBytes = stride;
		m_BufferView.SizeInBytes = size;

		CreateBuffer(&m_UploadResource, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	Dx12VertexBuffer::Dx12VertexBuffer(const float* vertices, uint32_t size, uint32_t stride)
	{
		CreateBuffer(&m_Resource, size, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
		m_BufferView.BufferLocation = m_Resource->GetGPUVirtualAddress();
		m_BufferView.StrideInBytes = stride;
		m_BufferView.SizeInBytes = size;

		CreateBuffer(&m_UploadResource, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
		SetBufferData(m_UploadResource, vertices, size);
		Dx12Context::GetUploadCommandList()->CopyBufferRegion(m_Resource, 0, m_UploadResource, 0, size);
	}

	Dx12VertexBuffer::~Dx12VertexBuffer()
	{
		if (m_UploadResource)
			m_UploadResource->Release();
		if (m_Resource)
			m_Resource->Release();
	}

	void Dx12VertexBuffer::Bind() const
	{
		Dx12Context::GetGraphicsCommandList()->IASetVertexBuffers(0, 1, &m_BufferView);
	}

	void Dx12VertexBuffer::Unbind() const
	{
	}

	void Dx12VertexBuffer::SetData(const void* data, uint32_t size)
	{
		const auto toCommonBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		Dx12Context::GetUploadCommandList()->ResourceBarrier(1, &toCommonBarrier);

		SetBufferData(m_UploadResource, data, size);
		Dx12Context::GetUploadCommandList()->CopyBufferRegion(m_Resource, 0, m_UploadResource, 0, size);

		const auto toVertexBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		Dx12Context::GetUploadCommandList()->ResourceBarrier(1, &toVertexBarrier);
	}


	//////////////////////////////////////////////////////////////////////
	// IndexBuffer ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	Dx12IndexBuffer::Dx12IndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		const uint32_t size = count * sizeof(uint32_t);

		CreateBuffer(&m_Resource, size, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
		m_BufferView.BufferLocation = m_Resource->GetGPUVirtualAddress();
		m_BufferView.Format = DXGI_FORMAT_R32_UINT;
		m_BufferView.SizeInBytes = size;

		CreateBuffer(&m_UploadResource, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
		SetBufferData(m_UploadResource, indices, size);
		Dx12Context::GetUploadCommandList()->CopyBufferRegion(m_Resource, 0, m_UploadResource, 0, size);
	}

	Dx12IndexBuffer::~Dx12IndexBuffer()
	{
		if (m_UploadResource)
			m_UploadResource->Release();
		if (m_Resource)
			m_Resource->Release();
	}

	void Dx12IndexBuffer::Bind() const
	{
		Dx12Context::GetGraphicsCommandList()->IASetIndexBuffer(&m_BufferView);
	}

	void Dx12IndexBuffer::Unbind() const
	{
	}
}
