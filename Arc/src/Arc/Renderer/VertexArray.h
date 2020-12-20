#pragma once

#include <memory>
#include "Arc/Renderer/Buffer.h"

namespace ArcEngine
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;
		
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffer() const = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;
		
		static VertexArray* Create();
	};
}
