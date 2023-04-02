#include "arcpch.h"
#include "Dx12Allocator.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <D3D12MemAlloc.h>

#include "Dx12Context.h"
#include "DxHelper.h"

namespace ArcEngine
{
	inline static D3D12MA::Allocator* s_Allocator = nullptr;

	void Dx12Allocator::Init(IDXGIAdapter* adapter, ID3D12Device* device)
	{
		D3D12MA::ALLOCATOR_DESC desc = {};
		desc.Flags = D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
		desc.pDevice = device;
		desc.pAdapter = adapter;
		ThrowIfFailed(D3D12MA::CreateAllocator(&desc, &s_Allocator), "Failed to create D3D12 Memory Allocator!");
	}

	void Dx12Allocator::Shutdown()
	{
		s_Allocator->Release();
	}

	void Dx12Allocator::CreateTextureResource(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* resourceDesc,
		D3D12_RESOURCE_STATES initialState, D3D12MA::Allocation** outAllocation)
	{
		ARC_CORE_ASSERT(outAllocation, "D3D12MA::Allocation** cannot be null!")

		D3D12MA::ALLOCATION_DESC defaultAllocation{};
		defaultAllocation.HeapType = heapType;

		s_Allocator->CreateResource(&defaultAllocation, resourceDesc, initialState, nullptr, outAllocation, IID_NULL, nullptr);
	}

	void Dx12Allocator::CreateBufferResource(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* resourceDesc,
		D3D12_RESOURCE_STATES initialState, D3D12MA::Allocation** outAllocation)
	{
		ARC_CORE_ASSERT(outAllocation, "D3D12MA::Allocation** cannot be null!")

		D3D12MA::ALLOCATION_DESC defaultAllocation{};
		defaultAllocation.HeapType = heapType;

		s_Allocator->CreateResource(&defaultAllocation, resourceDesc, initialState, nullptr, outAllocation, IID_NULL, nullptr);
	}

	void Dx12Allocator::CreateRtvResource(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* resourceDesc,
		D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE* clearValue, D3D12MA::Allocation** outAllocation)
	{
		ARC_CORE_ASSERT(outAllocation, "D3D12MA::Allocation** cannot be null!")

		D3D12MA::ALLOCATION_DESC defaultAllocation{};
		defaultAllocation.HeapType = heapType;

		s_Allocator->CreateResource(&defaultAllocation, resourceDesc, initialState, clearValue, outAllocation, IID_NULL, nullptr);
	}
}
