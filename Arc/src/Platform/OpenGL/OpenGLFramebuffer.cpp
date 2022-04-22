#include "arcpch.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace ArcEngine
{
	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			OPTICK_EVENT();

			glCreateTextures(TextureTarget(multisampled), count, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			OPTICK_EVENT();

			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
		{
			OPTICK_EVENT();

			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			OPTICK_EVENT();

			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case ArcEngine::FramebufferTextureFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}
	}
	
	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		OPTICK_EVENT();

		for (auto& spec : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColorAttachmentSpecifications.emplace_back(spec);
			else
				m_DepthAttachmentSpecification = spec;
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		OPTICK_EVENT();

		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFramebuffer::Invalidate()
	{
		OPTICK_EVENT();

		if(m_RendererID)
		{
			{
				OPTICK_EVENT("FramebufferDelete");

				glDeleteFramebuffers(1, &m_RendererID);
			}

			{
				OPTICK_EVENT("FramebufferAttachmentDelete");

				OPTICK_TAG("ColorAttachmentsSize", m_ColorAttachments.size());

				glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
				glDeleteTextures(1, &m_DepthAttachment);
			}

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}
		
		{
			OPTICK_EVENT("FramebufferCreate");
			
			glCreateFramebuffers(1, &m_RendererID);
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		}

		{
			OPTICK_EVENT("FramebufferConfigureAttachments");

			bool multisample = m_Specification.Samples > 1;
			// Attachments
			if (m_ColorAttachmentSpecifications.size())
			{
				m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
				Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());

				for (size_t i = 0; i < m_ColorAttachments.size(); i++)
				{
					Utils::BindTexture(multisample, m_ColorAttachments[i]);
					switch (m_ColorAttachmentSpecifications[i].TextureFormat)
					{
						case FramebufferTextureFormat::RGBA8:
							Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
							break;
						case FramebufferTextureFormat::RGBA16F:
							Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA16F, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
							break;
						case FramebufferTextureFormat::R11G11B10:
							Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R11F_G11F_B10F, GL_RGB, m_Specification.Width, m_Specification.Height, i);
							break;
						case FramebufferTextureFormat::R32I:
							Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, i);
							break;
					}
				}
			}

			if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
			{
				Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
				Utils::BindTexture(multisample, m_DepthAttachment);
				switch (m_DepthAttachmentSpecification.TextureFormat)
				{
					case FramebufferTextureFormat::DEPTH24STENCIL8:
						Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
						break;
				}
			}

			if (m_ColorAttachmentSpecifications.size() > 1)
			{
				ARC_CORE_ASSERT(m_ColorAttachments.size() <= 4);
				GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
				glDrawBuffers(m_ColorAttachments.size(), buffers);
			}
			else if (m_ColorAttachments.empty())
			{
				// Only Depth-Pass
				glDrawBuffer(GL_NONE);
			}

			ARC_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		OPTICK_EVENT();

		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFramebuffer::Unbind()
	{
		OPTICK_EVENT();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::BindColorAttachment(uint32_t index, uint32_t slot)
	{
		OPTICK_EVENT();

		ARC_CORE_ASSERT(index < m_ColorAttachments.size());
		glBindTextureUnit(slot, m_ColorAttachments[index]);
	}

	void OpenGLFramebuffer::BindDepthAttachment(uint32_t slot)
	{
		OPTICK_EVENT();

		glBindTextureUnit(slot, m_DepthAttachment);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		OPTICK_EVENT();

		if(width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			ARC_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}
		
		m_Specification.Width = width;
		m_Specification.Height = height;
		
		Invalidate();
	}
}
