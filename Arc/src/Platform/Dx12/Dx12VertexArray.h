#pragma once

#include "Arc/Renderer/VertexArray.h"

namespace ArcEngine
{
	class Dx12VertexArray : public VertexArray
	{
	public:
		Dx12VertexArray();
		~Dx12VertexArray() override;

		Dx12VertexArray(const Dx12VertexArray& other) = default;
		Dx12VertexArray(Dx12VertexArray&& other) = default;

		void Bind() const override;
		void AddVertexBuffer(Ref<VertexBuffer>& vertexBuffer) override;
		void SetIndexBuffer(Ref<IndexBuffer>& indexBuffer) override;
		[[nodiscard]] const std::vector<Ref<VertexBuffer>>& GetVertexBuffer() const override { return m_VertexBuffers; }
		[[nodiscard]] const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
