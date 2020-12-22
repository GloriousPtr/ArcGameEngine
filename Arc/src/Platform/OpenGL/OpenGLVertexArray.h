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
		virtual void AddVertexBuffer(Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(Ref<IndexBuffer>& indexBuffer) override;
		const std::vector<Ref<VertexBuffer>>& GetVertexBuffer() const override { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
