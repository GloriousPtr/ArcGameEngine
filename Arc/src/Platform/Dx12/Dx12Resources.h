#pragma once

#include "Arc/Core/Base.h"
#include "Dx12Context.h"

#include <d3d12.h>
#include <mutex>

namespace ArcEngine
{
	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE			CPU{};
		D3D12_GPU_DESCRIPTOR_HANDLE			GPU{};

		[[nodiscard]] constexpr bool IsValid() const { return CPU.ptr != 0; }
		[[nodiscard]] constexpr bool IsShaderVisible() const { return GPU.ptr != 0; }
	};

	class DescriptorHeap
	{
	public:
		explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) : m_Type(type) {}
		~DescriptorHeap() { ARC_CORE_ASSERT(!m_Heap); }

		bool Init(uint32_t capacity, bool shaderVisible);
		void Release();
		void ProcessDeferredFree(uint32_t frameIndex);

		[[nodiscard]] DescriptorHandle Allocate();
		void Free(DescriptorHandle& handle);

		[[nodiscard]] constexpr D3D12_DESCRIPTOR_HEAP_TYPE Type() const { return m_Type; }
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE CpuStart() const { return m_CpuStart; }
		[[nodiscard]] constexpr D3D12_GPU_DESCRIPTOR_HANDLE GpuStart() const { return m_GpuStart; }
		[[nodiscard]] ID3D12DescriptorHeap* Heap() const { return m_Heap; }
		[[nodiscard]] constexpr uint32_t Capacity() const { return m_Capacity; }
		[[nodiscard]] constexpr uint32_t Size() const { return m_Size; }
		[[nodiscard]] constexpr uint32_t DescriptorSize() const { return m_DescriptorSize; }
		[[nodiscard]] constexpr bool IsShaderVisible() const { return m_GpuStart.ptr != 0; }

	private:
		ID3D12DescriptorHeap*				m_Heap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE			m_CpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE			m_GpuStart{};
		Scope<uint32_t[]>					m_FreeHandles{};
		std::vector<uint32_t>				m_DeferedFreeIndices[Dx12Context::FrameCount]{};
		std::mutex							m_Mutex{};
		uint32_t							m_Capacity = 0;
		uint32_t							m_Size = 0;
		uint32_t							m_DescriptorSize = 0;
		const D3D12_DESCRIPTOR_HEAP_TYPE	m_Type{};
	};
}
