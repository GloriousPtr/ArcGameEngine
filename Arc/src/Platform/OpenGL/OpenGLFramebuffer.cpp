#include "arcpch.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace ArcEngine
{
	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			ARC_PROFILE_SCOPE()

			glCreateTextures(TextureTarget(multisampled), static_cast<int>(count), outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			ARC_PROFILE_SCOPE()

			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
		{
			ARC_PROFILE_SCOPE()

			const bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, static_cast<int>(width), static_cast<int>(height), GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(internalFormat), static_cast<int>(width), static_cast<int>(height), 0, format, GL_UNSIGNED_BYTE, nullptr);

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
			ARC_PROFILE_SCOPE()

			const bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, static_cast<int>(width), static_cast<int>(height), GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, static_cast<int>(width), static_cast<int>(height));

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
			return format == FramebufferTextureFormat::DEPTH24STENCIL8;
		}
	}
	
	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		ARC_PROFILE_SCOPE()

		for (const auto& s : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(s.TextureFormat))
				m_ColorAttachmentSpecifications.emplace_back(s);
			else
				m_DepthAttachmentSpecification = s;
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		ARC_PROFILE_SCOPE()

		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(static_cast<int>(m_ColorAttachments.size()), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFramebuffer::Invalidate()
	{
		ARC_PROFILE_SCOPE()

		if(m_RendererID)
		{
			ARC_PROFILE_SCOPE("FramebufferDelete")

			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(static_cast<int>(m_ColorAttachments.size()), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}
		
		{
			ARC_PROFILE_SCOPE("FramebufferCreate")
			
			glCreateFramebuffers(1, &m_RendererID);
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		}

		{
			ARC_PROFILE_SCOPE("FramebufferConfigureAttachments")

			const bool multisample = m_Specification.Samples > 1;
			// Attachments
			if (!m_ColorAttachmentSpecifications.empty())
			{
				m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
				const auto colorAttachmentSize = static_cast<int32_t>(m_ColorAttachments.size());
				Utils::CreateTextures(multisample, m_ColorAttachments.data(), colorAttachmentSize);

				for (int i = 0; i < colorAttachmentSize; ++i)
				{
					Utils::BindTexture(multisample, m_ColorAttachments[i]);
					switch (m_ColorAttachmentSpecifications[i].TextureFormat)
					{
						case FramebufferTextureFormat::RGBA8:
							Utils::AttachColorTexture(m_ColorAttachments[i], static_cast<int>(m_Specification.Samples), GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
							break;
						case FramebufferTextureFormat::RGBA16F:
							Utils::AttachColorTexture(m_ColorAttachments[i], static_cast<int>(m_Specification.Samples), GL_RGBA16F, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
							break;
						case FramebufferTextureFormat::RGBA32F:
							Utils::AttachColorTexture(m_ColorAttachments[i], static_cast<int>(m_Specification.Samples), GL_RGBA32F, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
							break;
						case FramebufferTextureFormat::R11G11B10F:
							Utils::AttachColorTexture(m_ColorAttachments[i], static_cast<int>(m_Specification.Samples), GL_R11F_G11F_B10F, GL_RGB, m_Specification.Width, m_Specification.Height, i);
							break;
						case FramebufferTextureFormat::RG16F:
							Utils::AttachColorTexture(m_ColorAttachments[i], static_cast<int>(m_Specification.Samples), GL_RG16F, GL_RG, m_Specification.Width, m_Specification.Height, i);
							break;
						case FramebufferTextureFormat::R32I:
							Utils::AttachColorTexture(m_ColorAttachments[i], static_cast<int>(m_Specification.Samples), GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, i);
							break;
						default:
							ARC_CORE_ASSERT(false)
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
						Utils::AttachDepthTexture(m_DepthAttachment, static_cast<int>(m_Specification.Samples), GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
						break;
					default:
						ARC_CORE_ASSERT(false)
						break;
				}
			}

			if (m_ColorAttachmentSpecifications.size() > 1)
			{
				ARC_CORE_ASSERT(m_ColorAttachments.size() <= 5)
				constexpr GLenum buffers[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
				glDrawBuffers(static_cast<int>(m_ColorAttachments.size()), buffers);
			}
			else if (m_ColorAttachments.empty())
			{
				// Only Depth-Pass
				glDrawBuffer(GL_NONE);
			}

			ARC_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!")
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		ARC_PROFILE_SCOPE()

		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, static_cast<int>(m_Specification.Width), static_cast<int>(m_Specification.Height));
	}

	void OpenGLFramebuffer::Unbind()
	{
		ARC_PROFILE_SCOPE()

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::BindColorAttachment(uint32_t index, uint32_t slot)
	{
		ARC_PROFILE_SCOPE()

		ARC_CORE_ASSERT(index < m_ColorAttachments.size())
		glBindTextureUnit(slot, m_ColorAttachments[index]);
	}

	void OpenGLFramebuffer::BindDepthAttachment(uint32_t slot)
	{
		ARC_PROFILE_SCOPE()

		glBindTextureUnit(slot, m_DepthAttachment);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE()

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
