#pragma once

struct IDXGIAdapter;
struct ID3D12Device;

#include <D3D12MemAlloc.h>

namespace ArcEngine
{
	class Dx12Allocator
	{
	public:
		static void Init(IDXGIAdapter* adapter, ID3D12Device* device);
		static void Shutdown();

		static void CreateTextureResource(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* resourceDesc, D3D12_RESOURCE_STATES initialState, D3D12MA::Allocation** outAllocation);
		static void CreateBufferResource(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* resourceDesc, D3D12_RESOURCE_STATES initialState, D3D12MA::Allocation** outAllocation);
		static void CreateRtvResource(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* resourceDesc, D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE* clearValue, D3D12MA::Allocation** outAllocation);

		static void GetStats(D3D12MA::Budget& budget);
	};
}
