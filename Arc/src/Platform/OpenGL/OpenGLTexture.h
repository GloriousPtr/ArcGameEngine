#pragma once

#include "Arc/Renderer/Texture.h"

#include <glad/glad.h>

namespace ArcEngine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D() = default;
		OpenGLTexture2D(uint32_t width, uint32_t height);
		explicit OpenGLTexture2D(const std::string& path);
		~OpenGLTexture2D() override;

		OpenGLTexture2D(const OpenGLTexture2D& other) = default;
		OpenGLTexture2D(OpenGLTexture2D&& other) = default;

		[[nodiscard]] uint32_t GetWidth() const override { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const override { return m_Height; }
		[[nodiscard]] uint32_t GetRendererID() const override { return m_RendererID; }
		[[nodiscard]] const std::string& GetPath() const override { return m_Path; }

		void SetData(void* data, [[maybe_unused]] uint32_t size) override;
		void Invalidate(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels) override;

		void Bind(uint32_t slot = 0) const override;

	private:
		void InvalidateImpl(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels);

	private:
		std::string m_Path;
		uint32_t m_Width = 0, m_Height = 0;
		uint32_t m_RendererID = 0;
		GLenum m_InternalFormat = 0, m_DataFormat = 0;
	};
}
