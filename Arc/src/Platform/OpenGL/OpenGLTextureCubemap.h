#pragma once

#include "Arc/Renderer/Texture.h"

#include <glad/glad.h>

namespace ArcEngine
{
	class OpenGLTextureCubemap : public TextureCubemap
	{
	public:
		OpenGLTextureCubemap(const eastl::string& path);
		virtual ~OpenGLTextureCubemap();
		
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint64_t GetRendererID() const override { return m_RendererID; }
		virtual uint64_t GetHRDRendererID() const override { return m_HRDRendererID; }
		virtual const eastl::string& GetPath() const override { return m_Path; }

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void BindIrradianceMap(uint32_t slot) const override;
		virtual void BindRadianceMap(uint32_t slot) const override;

		virtual bool operator==(const Texture& other) const override { return m_RendererID == ((OpenGLTextureCubemap&)other).m_RendererID; }
	private:
		eastl::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_HRDRendererID;
		uint32_t m_RendererID;
		uint32_t m_IrradianceRendererID;
		uint32_t m_RadianceRendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}

