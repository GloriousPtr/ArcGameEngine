#pragma once

#include "Arc/Renderer/VertexArray.h"

namespace ArcEngine
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray() override;

		OpenGLVertexArray(const OpenGLVertexArray& other) = default;
		OpenGLVertexArray(OpenGLVertexArray&& other) = default;

		void Bind() const override;
		void Unbind() const override;
		void AddVertexBuffer(Ref<VertexBuffer>& vertexBuffer) override;
		void SetIndexBuffer(Ref<IndexBuffer>& indexBuffer) override;
		const std::vector<Ref<VertexBuffer>>& GetVertexBuffer() const override { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
