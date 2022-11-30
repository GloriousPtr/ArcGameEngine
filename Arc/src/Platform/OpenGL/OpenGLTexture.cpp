#include "arcpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include <stb_image.h>

namespace ArcEngine
{
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		ARC_PROFILE_SCOPE();
		
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;
		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const eastl::string& path)
	{
		ARC_PROFILE_SCOPE();

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			ARC_PROFILE_SCOPE("stbi_load Texture");
			
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		ARC_CORE_ASSERT(data, "Failed to load image!");
		
		InvalidateImpl(path, width, height, data, channels);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		ARC_PROFILE_SCOPE();
		
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		ARC_PROFILE_SCOPE();
		
		ARC_CORE_ASSERT(size == m_Width * m_Height * (m_DataFormat == GL_RGBA ? 4 : 3), "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Invalidate(const eastl::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels)
	{
		ARC_PROFILE_SCOPE();

		InvalidateImpl(path, width, height, data, channels);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		ARC_PROFILE_SCOPE();
		
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::InvalidateImpl(const eastl::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels)
	{
		ARC_PROFILE_SCOPE();

		m_Path = path;

		if (m_RendererID)
			glDeleteTextures(1, &m_RendererID);

		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		switch (channels)
		{
			case 1:
				internalFormat = GL_R8;
				dataFormat = GL_RED;
				break;
			case 2:
				internalFormat = GL_RG8;
				dataFormat = GL_RG;
				break;
			case 3:
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
				break;
			case 4:
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
				break;
			default:
				ARC_CORE_ERROR("Texture channel count is not within (1-4) range. Channel count: {}", channels);
				break;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		ARC_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}
