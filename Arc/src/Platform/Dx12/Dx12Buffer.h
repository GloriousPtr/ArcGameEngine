#pragma once

#include "Arc/Renderer/Buffer.h"
#include "Platform/Dx12/Dx12Resources.h"

namespace D3D12MA
{
	class Allocation;
}

namespace ArcEngine
{
	class Dx12VertexBuffer : public VertexBuffer
	{
	public:
		Dx12VertexBuffer(uint32_t size, uint32_t stride);
		Dx12VertexBuffer(const float* vertices, uint32_t size, uint32_t stride);
		~Dx12VertexBuffer() override;

		Dx12VertexBuffer(const Dx12VertexBuffer& other) = default;
		Dx12VertexBuffer(Dx12VertexBuffer&& other) = default;

		void Bind() const override;

		void SetData(const void* data, uint32_t size) override;

	private:
		D3D12MA::Allocation*				m_UploadAllocation = nullptr;
		D3D12MA::Allocation*				m_Allocation = nullptr;
		D3D12_VERTEX_BUFFER_VIEW			m_BufferView{};
	};

	class Dx12IndexBuffer : public IndexBuffer
	{
	public:
		Dx12IndexBuffer(const uint32_t* indices, uint32_t count);
		~Dx12IndexBuffer() override;

		Dx12IndexBuffer(const Dx12IndexBuffer& other) = default;
		Dx12IndexBuffer(Dx12IndexBuffer&& other) = default;

		void Bind() const override;

		[[nodiscard]] uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t							m_Count = 0;
		D3D12MA::Allocation*				m_UploadAllocation = nullptr;
		D3D12MA::Allocation*				m_Allocation = nullptr;
		D3D12_INDEX_BUFFER_VIEW				m_BufferView{};
	};

	class Dx12ConstantBuffer : public ConstantBuffer
	{
	public:
		Dx12ConstantBuffer(uint32_t size, uint32_t count, uint32_t registerIndex);
		~Dx12ConstantBuffer() override;

		void SetData(const void* data, uint32_t size, uint32_t index) override;
		void Bind(uint32_t index) const override;

	private:
		DescriptorHandle				m_Handle[Dx12Context::FrameCount]{};
		D3D12MA::Allocation*			m_Allocation[Dx12Context::FrameCount]{};
		uint32_t						m_RegisterIndex = 0;
		uint32_t						m_Size = 0;
		uint32_t						m_Count = 0;
		uint32_t						m_AlignedSize = 0;
	};
}
