#include "arcpch.h"
#include "Dx12Resources.h"

namespace ArcEngine
{
	bool DescriptorHeap::Init(uint32_t capacity, bool shaderVisible)
	{
		std::lock_guard lock(m_Mutex);

		if (m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ||
			m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			shaderVisible = false;
		}

		ARC_CORE_ASSERT(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		ARC_CORE_ASSERT(!(m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));

		if (m_Size)
			Release();

		auto* device = Dx12Context::GetDevice();
		ARC_CORE_ASSERT(device);

		D3D12_DESCRIPTOR_HEAP_DESC desc {};
		desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = capacity;
		desc.Type = m_Type;
		desc.NodeMask = 0;

		const HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_Heap));
		if (FAILED(hr))
			return false;

		m_FreeHandles = CreateScope<uint32_t[]>(capacity);
		m_Capacity = capacity;
		m_Size = 0;

		for (uint32_t i = 0; i < capacity; ++i)
			m_FreeHandles[i] = i;

		for ([[maybe_unused]] const auto& deferedFreeIndices : m_DeferedFreeIndices)
		{
			ARC_CORE_ASSERT(deferedFreeIndices.empty());
		}

		m_DescriptorSize = device->GetDescriptorHandleIncrementSize(m_Type);
		m_CpuStart = m_Heap->GetCPUDescriptorHandleForHeapStart();
		m_GpuStart = shaderVisible ? m_Heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}

	void DescriptorHeap::Release()
	{
		ARC_CORE_ASSERT(!m_Size);
		Dx12Context::DeferredRelease(m_Heap);
		m_Heap = nullptr;
	}

	void DescriptorHeap::ProcessDeferredFree(uint32_t frameIndex)
	{
		std::lock_guard lock(m_Mutex);
		ARC_CORE_ASSERT(frameIndex < Dx12Context::FrameCount);

		auto& indices = m_DeferedFreeIndices[frameIndex];
		for (const auto index : indices)
		{
			--m_Size;
			m_FreeHandles[m_Size] = index;
		}
		indices.clear();
	}

	DescriptorHandle DescriptorHeap::Allocate()
	{
		std::lock_guard lock(m_Mutex);

		ARC_CORE_ASSERT(m_Heap);
		ARC_CORE_ASSERT(m_Size < m_Capacity);

		const uint32_t index = m_FreeHandles[m_Size];
		const uint32_t offset = index * m_DescriptorSize;
		++m_Size;

		DescriptorHandle handle;
		handle.CPU.ptr = m_CpuStart.ptr + offset;
		if (IsShaderVisible())
			handle.GPU.ptr = m_GpuStart.ptr + offset;

		return handle;
	}

	void DescriptorHeap::Free(DescriptorHandle& handle)
	{
		if (!handle.IsValid())
			return;

		std::lock_guard lock(m_Mutex);

		ARC_CORE_ASSERT(m_Heap && m_Size);
		ARC_CORE_ASSERT(handle.CPU.ptr >= m_CpuStart.ptr);
		ARC_CORE_ASSERT(handle.GPU.ptr >= m_GpuStart.ptr);
		ARC_CORE_ASSERT((handle.CPU.ptr - m_CpuStart.ptr) % m_DescriptorSize == 0);
		ARC_CORE_ASSERT(m_Heap && m_Size);

		const uint32_t index = static_cast<uint32_t>(handle.CPU.ptr - m_CpuStart.ptr) / m_DescriptorSize;
		ARC_CORE_ASSERT(index < m_Capacity);

		const uint32_t frameIndex = Dx12Context::GetCurrentFrameIndex();
		m_DeferedFreeIndices[frameIndex].push_back(index);
		Dx12Context::SetDeferredReleaseFlag();

		handle = {};
	}
}
