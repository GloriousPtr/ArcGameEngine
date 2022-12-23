#pragma once

#include "Arc/Renderer/Framebuffer.h"

namespace ArcEngine
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		explicit OpenGLFramebuffer(const FramebufferSpecification& spec);
		~OpenGLFramebuffer() override;

		OpenGLFramebuffer(const OpenGLFramebuffer& other) = default;
		OpenGLFramebuffer(OpenGLFramebuffer&& other) = default;
		
		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void BindColorAttachment(uint32_t index, uint32_t slot) override;
		void BindDepthAttachment(uint32_t slot) override;

		void Resize(uint32_t width, uint32_t height) override;

		[[nodiscard]] uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override { ARC_CORE_ASSERT(index < m_ColorAttachments.size()) return m_ColorAttachments[index]; }
		[[nodiscard]] uint64_t GetDepthAttachmentRendererID() const override { return m_DepthAttachment; }

		[[nodiscard]] const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;
		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment;
	};
}
