#pragma once

#include "Arc/Renderer/Texture.h"

#include <glad/glad.h>

namespace ArcEngine
{
	class OpenGLTextureCubemap : public TextureCubemap
	{
	public:
		OpenGLTextureCubemap() = default;
		explicit OpenGLTextureCubemap(const std::string& path);
		~OpenGLTextureCubemap() override;

		OpenGLTextureCubemap(const OpenGLTextureCubemap& other) = default;
		OpenGLTextureCubemap(OpenGLTextureCubemap&& other) = default;

		[[nodiscard]] uint32_t GetWidth() const override { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const override { return m_Height; }
		[[nodiscard]] uint64_t GetRendererID() const override { return m_RendererID; }
		[[nodiscard]] uint32_t GetIndex() const override { return 0; }
		[[nodiscard]] uint64_t GetHRDRendererID() const override { return m_HRDRendererID; }
		[[nodiscard]] const std::string& GetPath() const override { return m_Path; }

		void SetData(const TextureData data, uint32_t size) override;
		void Invalidate(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels) override;

		void Bind(uint32_t slot = 0) const override;
		void BindIrradianceMap(uint32_t slot) const override;
		void BindRadianceMap(uint32_t slot) const override;

	private:
		void InvalidateImpl(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels);

	private:
		std::string m_Path;
		uint32_t m_Width = 0, m_Height = 0;
		uint32_t m_HRDRendererID = 0;
		uint32_t m_RendererID = 0;
		uint32_t m_IrradianceRendererID = 0;
		uint32_t m_RadianceRendererID = 0;
		GLenum m_InternalFormat = 0, m_DataFormat = 0;
	};
}
