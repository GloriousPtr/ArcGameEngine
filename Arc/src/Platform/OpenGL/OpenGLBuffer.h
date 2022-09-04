#pragma once

#include "Arc/Renderer/Buffer.h"

namespace ArcEngine
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		explicit OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(const float* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		OpenGLVertexBuffer(const OpenGLVertexBuffer& other) = default;
		OpenGLVertexBuffer(OpenGLVertexBuffer&& other) = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		
		virtual void SetData(const BufferData data, uint32_t size) override;
		
		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		OpenGLIndexBuffer(const OpenGLIndexBuffer& other) = default;
		OpenGLIndexBuffer(OpenGLIndexBuffer&& other) = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer();
		virtual ~OpenGLUniformBuffer();

		OpenGLUniformBuffer(const OpenGLUniformBuffer& other) = default;
		OpenGLUniformBuffer(OpenGLUniformBuffer&& other) = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const BufferData data, uint32_t offset, uint32_t size) override;
		virtual void SetLayout(const BufferLayout& layout, uint32_t blockIndex, uint32_t count) override;

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};
}
