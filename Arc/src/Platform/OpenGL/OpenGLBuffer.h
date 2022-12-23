#pragma once

#include "Arc/Renderer/Buffer.h"

namespace ArcEngine
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		explicit OpenGLVertexBuffer(size_t size);
		OpenGLVertexBuffer(const float* vertices, size_t size);
		~OpenGLVertexBuffer() override;

		OpenGLVertexBuffer(const OpenGLVertexBuffer& other) = default;
		OpenGLVertexBuffer(OpenGLVertexBuffer&& other) = default;

		void Bind() const override;
		void Unbind() const override;

		void SetData(const void* data, uint32_t size) override;

		[[nodiscard]] const BufferLayout& GetLayout() const override { return m_Layout; }
		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

	private:
		uint32_t m_RendererID = 0;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
		~OpenGLIndexBuffer() override;

		OpenGLIndexBuffer(const OpenGLIndexBuffer& other) = default;
		OpenGLIndexBuffer(OpenGLIndexBuffer&& other) = default;

		void Bind() const override;
		void Unbind() const override;

		[[nodiscard]] uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Count = 0;
	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer();
		~OpenGLUniformBuffer() override;

		OpenGLUniformBuffer(const OpenGLUniformBuffer& other) = default;
		OpenGLUniformBuffer(OpenGLUniformBuffer&& other) = default;

		void Bind() const override;
		void Unbind() const override;

		void SetData(const void* data, uint32_t offset, uint32_t size) override;
		void SetLayout(const BufferLayout& layout, uint32_t blockIndex, uint32_t count) override;

	private:
		uint32_t m_RendererID = 0;
		BufferLayout m_Layout;
	};
}
