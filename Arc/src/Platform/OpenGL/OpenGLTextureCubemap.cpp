#include "arcpch.h"
#include "OpenGLTextureCubemap.h"

#include "Arc/Renderer/Renderer3D.h"
#include "Arc/Renderer/Shader.h"

#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ArcEngine
{
	static Ref<Shader> s_EquirectangularToCubemapShader;
	static Ref<Shader> s_IrradianceShader;
	static Ref<Shader> s_RadianceShader;

	OpenGLTextureCubemap::OpenGLTextureCubemap(const std::string& path)
	{
		ARC_PROFILE_SCOPE()
		
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		float* data = nullptr;
		{
			ARC_PROFILE_SCOPE("stbi_load Texture")
			
			data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
		}
		ARC_CORE_ASSERT(data, "Failed to load image!")
		
		InvalidateImpl(path, width, height, data, channels);

		stbi_image_free(data);
	}

	OpenGLTextureCubemap::~OpenGLTextureCubemap()
	{
		ARC_PROFILE_SCOPE()
		
		glDeleteTextures(1, &m_HRDRendererID);
		glDeleteTextures(1, &m_RendererID);
		glDeleteTextures(1, &m_IrradianceRendererID);
		glDeleteTextures(1, &m_RadianceRendererID);
	}

	void OpenGLTextureCubemap::SetData([[maybe_unused]] const TextureData data, [[maybe_unused]] uint32_t size)
	{
		ARC_PROFILE_SCOPE()
		
	}

	void OpenGLTextureCubemap::Invalidate(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels)
	{
		ARC_PROFILE_SCOPE()

		InvalidateImpl(path, width, height, data, channels);
	}

	void OpenGLTextureCubemap::Bind(uint32_t slot) const
	{
		ARC_PROFILE_SCOPE()
		
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTextureCubemap::BindIrradianceMap(uint32_t slot) const
	{
		ARC_PROFILE_SCOPE()

		glBindTextureUnit(slot, m_IrradianceRendererID);
	}

	void OpenGLTextureCubemap::BindRadianceMap(uint32_t slot) const
	{
		ARC_PROFILE_SCOPE()

		glBindTextureUnit(slot, m_RadianceRendererID);
	}

	void OpenGLTextureCubemap::InvalidateImpl(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels)
	{
		ARC_PROFILE_SCOPE()

		if (channels < 3)
		{
			ARC_CORE_ERROR("Couldn't load HDR cubemap with {} channels: {}", channels, path);
			return;
		}

		m_Path = path;

		constexpr uint32_t cubemapSize = 2048;
		constexpr uint32_t irradianceMapSize = 32;
		constexpr uint32_t radianceMapSize = cubemapSize / 4;

		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		switch (channels)
		{
			case 1:
				internalFormat = GL_R16F;
				dataFormat = GL_RED;
				break;
			case 2:
				internalFormat = GL_RG16F;
				dataFormat = GL_RG;
				break;
			case 3:
				internalFormat = GL_RGB16F;
				dataFormat = GL_RGB;
				break;
			case 4:
				internalFormat = GL_RGBA16F;
				dataFormat = GL_RGBA;
				break;
			default:
				ARC_CORE_ERROR("Texture channel count is not within (1-4) range. Channel count: {}", channels);
				break;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		ARC_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!")

		uint32_t captureFBO, captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubemapSize, cubemapSize);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		glGenTextures(1, &m_HRDRendererID);
		glBindTexture(GL_TEXTURE_2D, m_HRDRendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, static_cast<int>(m_Width), static_cast<int>(m_Height), 0, dataFormat, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, cubemapSize, cubemapSize, 0, dataFormat, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		if (!s_EquirectangularToCubemapShader)
			s_EquirectangularToCubemapShader = Shader::Create("assets/shaders/EquirectangularToCubemap.glsl");

		s_EquirectangularToCubemapShader->Bind();
		s_EquirectangularToCubemapShader->SetInt("u_EquirectangularMap", 0);
		s_EquirectangularToCubemapShader->SetMat4("u_Projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_HRDRendererID);

		glViewport(0, 0, cubemapSize, cubemapSize); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			s_EquirectangularToCubemapShader->SetMat4("u_View", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_RendererID, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Renderer3D::DrawCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glGenTextures(1, &m_IrradianceRendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceRendererID);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irradianceMapSize, irradianceMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceMapSize, irradianceMapSize);

		if (!s_IrradianceShader)
			s_IrradianceShader = Shader::Create("assets/shaders/Irradiance.glsl");

		s_IrradianceShader->Bind();
		s_IrradianceShader->SetInt("u_EnvironmentMap", 0);
		s_IrradianceShader->SetMat4("u_Projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		glViewport(0, 0, irradianceMapSize, irradianceMapSize); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			s_IrradianceShader->SetMat4("u_View", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_IrradianceRendererID, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Renderer3D::DrawCube();
		}

		// Radiance
		glGenTextures(1, &m_RadianceRendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RadianceRendererID);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, radianceMapSize, radianceMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		if (!s_RadianceShader)
			s_RadianceShader = Shader::Create("assets/shaders/PrefilterCubemap.glsl");
		s_RadianceShader->Bind();
		s_RadianceShader->SetInt("u_EnvironmentMap", 0);
		s_RadianceShader->SetMat4("u_Projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		const int maxMipLevels = static_cast<int>(glm::log2(static_cast<float>(radianceMapSize))) + 1;
		constexpr auto radianceMapSizeFloat = static_cast<float>(radianceMapSize);
		for (int mip = 0; mip < maxMipLevels; ++mip)
		{
			const float m = radianceMapSizeFloat * static_cast<float>(glm::pow(0.5f, mip));
			const int sz = static_cast<int>(m);
			glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, sz, sz);
			glViewport(0, 0, sz, sz);

			const float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
			s_RadianceShader->SetFloat("u_Roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				s_RadianceShader->SetMat4("u_View", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_RadianceRendererID, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				Renderer3D::DrawCube();
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glDeleteFramebuffers(1, &captureFBO);
		glDeleteRenderbuffers(1, &captureRBO);
	}
}
