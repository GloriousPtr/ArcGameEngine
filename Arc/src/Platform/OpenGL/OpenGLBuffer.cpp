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
		ARC_PROFILE_SCOPE();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(const float* vertices, uint32_t size)
	{
		ARC_PROFILE_SCOPE();
		
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		ARC_PROFILE_SCOPE();
		
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		ARC_PROFILE_SCOPE();
		
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		ARC_PROFILE_SCOPE();
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		ARC_PROFILE_SCOPE();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}


	//////////////////////////////////////////////////////////////////////
	// IndexBuffer ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	
	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		ARC_PROFILE_SCOPE();
		
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		ARC_PROFILE_SCOPE();
		
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		ARC_PROFILE_SCOPE();
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		ARC_PROFILE_SCOPE();
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	//////////////////////////////////////////////////////////////////////
	// UniformBuffer /////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	
	OpenGLUniformBuffer::OpenGLUniformBuffer()
	{
		ARC_PROFILE_SCOPE();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		ARC_PROFILE_SCOPE();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::Bind() const
	{
		ARC_PROFILE_SCOPE();

		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
	}

	void OpenGLUniformBuffer::Unbind() const
	{
		ARC_PROFILE_SCOPE();

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t offset, uint32_t size)
	{
		ARC_PROFILE_SCOPE();

		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	}

	void OpenGLUniformBuffer::SetLayout(const BufferLayout& layout, uint32_t blockIndex, uint32_t count)
	{
		ARC_PROFILE_SCOPE();

		m_Layout = layout;

		uint32_t size = 0;
		for (const auto& element : m_Layout)
			size += ShaderDataTypeSize(element.Type);

		size *= count;

		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, blockIndex, m_RendererID, 0, size);
	}
}
