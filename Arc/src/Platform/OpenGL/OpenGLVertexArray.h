#pragma once

#include "Arc/Renderer/VertexArray.h"

namespace ArcEngine
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();
		
		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(std::shared_ptr<IndexBuffer>& indexBuffer) override;
		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffer() const override { return m_VertexBuffers; }
		const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};
}
