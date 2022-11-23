#pragma once

#include "Arc/Renderer/Texture.h"

#include <glad/glad.h>

namespace ArcEngine
{
	class OpenGLTextureCubemap : public TextureCubemap
	{
	public:
		OpenGLTextureCubemap() = default;
		explicit OpenGLTextureCubemap(const eastl::string& path);
		virtual ~OpenGLTextureCubemap();

		OpenGLTextureCubemap(const OpenGLTextureCubemap& other) = default;
		OpenGLTextureCubemap(OpenGLTextureCubemap&& other) = default;
		
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint64_t GetRendererID() const override { return m_RendererID; }
		virtual uint64_t GetHRDRendererID() const override { return m_HRDRendererID; }
		virtual const eastl::string& GetPath() const override { return m_Path; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual void Invalidate(const eastl::string_view path, uint32_t width, uint32_t height, void* data, uint32_t channels) override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void BindIrradianceMap(uint32_t slot) const override;
		virtual void BindRadianceMap(uint32_t slot) const override;

	private:
		void InvalidateImpl(const eastl::string_view path, uint32_t width, uint32_t height, void* data, uint32_t channels);

	private:
		eastl::string m_Path;
		uint32_t m_Width = 0, m_Height = 0;
		uint32_t m_HRDRendererID = 0;
		uint32_t m_RendererID = 0;
		uint32_t m_IrradianceRendererID = 0;
		uint32_t m_RadianceRendererID = 0;
		GLenum m_InternalFormat, m_DataFormat;
	};
}
