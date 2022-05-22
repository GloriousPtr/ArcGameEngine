#pragma once

#include "Arc/Renderer/Framebuffer.h"

namespace ArcEngine
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();
		
		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void BindColorAttachment(uint32_t index, uint32_t slot) override;
		virtual void BindDepthAttachment(uint32_t slot) override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { ARC_CORE_ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; }
		virtual uint32_t GetDepthAttachmentRendererID() const override { return m_DepthAttachment; }
		
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }
	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;
		eastl::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification;

		eastl::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment;
	};
}
