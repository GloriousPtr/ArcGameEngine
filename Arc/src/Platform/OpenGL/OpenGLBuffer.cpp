#include "arcpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include <glad/glad.h>

namespace ArcEngine
{
	
	//////////////////////////////////////////////////////////////////////
	// VertexBuffer //////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		ARC_PROFILE_SCOPE()

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(const float* vertices, uint32_t size)
	{
		ARC_PROFILE_SCOPE()
		
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		ARC_PROFILE_SCOPE()
		
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		ARC_PROFILE_SCOPE()
		
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		ARC_PROFILE_SCOPE()
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		ARC_PROFILE_SCOPE()

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}


	//////////////////////////////////////////////////////////////////////
	// IndexBuffer ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	
	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		ARC_PROFILE_SCOPE()
		
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(uint32_t)), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		ARC_PROFILE_SCOPE()
		
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		ARC_PROFILE_SCOPE()
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		ARC_PROFILE_SCOPE()
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	//////////////////////////////////////////////////////////////////////
	// UniformBuffer /////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	
	OpenGLConstantBuffer::OpenGLConstantBuffer(uint32_t size, uint32_t count, uint32_t registerIndex)
	{
		ARC_PROFILE_SCOPE()

		m_Size = size;

		size *= count;
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, registerIndex, m_RendererID, 0, size);
	}

	OpenGLConstantBuffer::~OpenGLConstantBuffer()
	{
		ARC_PROFILE_SCOPE()

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLConstantBuffer::Bind(uint32_t offset) const
	{
		ARC_PROFILE_SCOPE()

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_RendererID, offset, m_Size);
	}

	void OpenGLConstantBuffer::Unbind() const
	{
		ARC_PROFILE_SCOPE()

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLConstantBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		ARC_PROFILE_SCOPE()

		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	}
}
