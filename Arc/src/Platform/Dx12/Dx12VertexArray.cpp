#include "arcpch.h"
#include "Dx12VertexArray.h"

namespace ArcEngine
{
	Dx12VertexArray::Dx12VertexArray()
	{
	}

	Dx12VertexArray::~Dx12VertexArray()
	{
	}

	void Dx12VertexArray::Bind() const
	{
		m_VertexBuffers[0]->Bind();
		m_IndexBuffer->Bind();
	}

	void Dx12VertexArray::AddVertexBuffer(Ref<VertexBuffer>& vertexBuffer)
	{
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void Dx12VertexArray::SetIndexBuffer(Ref<IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
	}
}
