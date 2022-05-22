#pragma once

#include "Arc/Renderer/Buffer.h"

namespace ArcEngine
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;
		
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const eastl::vector<Ref<VertexBuffer>>& GetVertexBuffer() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;
		
		static Ref<VertexArray> Create();
	};
}
