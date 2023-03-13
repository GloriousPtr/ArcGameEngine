#pragma once

#include "Arc/Renderer/Buffer.h"
#include "Platform/Dx12/Dx12Resources.h"

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
		void Unbind() const override;

		void SetData(const void* data, uint32_t size) override;

		[[nodiscard]] const BufferLayout& GetLayout() const override { return m_Layout; }
		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

	private:
		BufferLayout						m_Layout{};
		ID3D12Resource*						m_UploadResource = nullptr;
		ID3D12Resource*						m_Resource = nullptr;
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
		void Unbind() const override;

		[[nodiscard]] uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t							m_Count = 0;
		ID3D12Resource*						m_UploadResource = nullptr;
		ID3D12Resource*						m_Resource = nullptr;
		D3D12_INDEX_BUFFER_VIEW				m_BufferView{};
	};
}
