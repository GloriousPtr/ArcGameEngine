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

	static void SetBufferData(ID3D12Resource* resource, const void* data, uint32_t size, uint32_t offset = 0)
	{
		constexpr D3D12_RANGE readRange{ 0, 0 };
		void* dst;
		resource->Map(0, &readRange, &dst);
		dst = (char*)dst + offset;
		memcpy(dst, data, size);
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




	Dx12ConstantBuffer::Dx12ConstantBuffer(uint32_t size, uint32_t count, uint32_t registerIndex)
	{
		m_RegisterIndex = registerIndex;
		m_Size = size;
		m_Count = count;
		m_AlignedSize = (size + 255) &~ 255;

		const uint32_t allocationSize = m_AlignedSize * count;

		const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(allocationSize);

		for (uint32_t i = 0; i < Dx12Context::FrameCount; ++i)
		{
			Dx12Context::GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_Resource[i]));

			D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
			desc.BufferLocation = m_Resource[i]->GetGPUVirtualAddress();
			desc.SizeInBytes = allocationSize;
			m_Handle[i] = Dx12Context::GetSrvHeap()->Allocate();
			Dx12Context::GetDevice()->CreateConstantBufferView(&desc, m_Handle[i].CPU);
		}
	}

	Dx12ConstantBuffer::~Dx12ConstantBuffer()
	{
		for (uint32_t i = 0; i < Dx12Context::FrameCount; ++i)
		{
			Dx12Context::GetSrvHeap()->Free(m_Handle[i]);

			if (m_Resource[i])
				m_Resource[i]->Release();
		}
	}

	void Dx12ConstantBuffer::SetData(const void* data, uint32_t size, uint32_t index)
	{
		ARC_CORE_ASSERT(m_Count > index, "Constant buffer index can't be greater than count! Overflow!")

		SetBufferData(m_Resource[Dx12Context::GetCurrentFrameIndex()], data, size, m_AlignedSize * index);
	}

	void Dx12ConstantBuffer::Bind(uint32_t index) const
	{
		ARC_CORE_ASSERT(m_Count > index, "Constant buffer index can't be greater than count! Overflow!")

		const auto gpuVirtualAddress = m_Resource[Dx12Context::GetCurrentFrameIndex()]->GetGPUVirtualAddress() + m_AlignedSize * index;
		Dx12Context::GetGraphicsCommandList()->SetGraphicsRootConstantBufferView(m_RegisterIndex, gpuVirtualAddress);
	}

	void Dx12ConstantBuffer::Unbind() const
	{
	}
}
