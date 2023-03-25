#pragma once

namespace ArcEngine
{
	using TextureData = void*;

	enum class TextureFormat
	{
		None = 0,

		RGBA32F,
		RGB32F,
		RG32F,
		R32F,

		RGBA16F,
		RG16F,
		R16F,

		RGBA8,
		RG8,
		R8,
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;
		
		[[nodiscard]] virtual uint32_t GetWidth() const = 0;
		[[nodiscard]] virtual uint32_t GetHeight() const = 0;
		[[nodiscard]] virtual TextureFormat GetTextureFormat() const = 0;
		[[nodiscard]] virtual uint64_t GetRendererID() const = 0;
		[[nodiscard]] virtual uint32_t GetIndex() const = 0;
		[[nodiscard]] virtual const std::string& GetPath() const = 0;

		virtual void SetData(const TextureData data, [[maybe_unused]] uint32_t size) = 0;
		virtual void Invalidate(std::string_view path, uint32_t width, uint32_t height, const void* data) = 0;
		
		virtual void Bind(uint32_t slot = 0) const = 0;

		bool operator==(const Texture& other) const { return GetRendererID() == other.GetRendererID(); }

		[[nodiscard]] static uint32_t ChannelCountFromFormat(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::None:		return 0;
				case TextureFormat::RGBA32F:	return 4;
				case TextureFormat::RGB32F:		return 3;
				case TextureFormat::RG32F:		return 2;
				case TextureFormat::R32F:		return 1;
				case TextureFormat::RGBA16F:	return 4;
				case TextureFormat::RG16F:		return 2;
				case TextureFormat::R16F:		return 1;
				case TextureFormat::RGBA8:		return 4;
				case TextureFormat::RG8:		return 2;
				case TextureFormat::R8:			return 1;
				default:						return 0;
			}
		}
	};

	class Texture2D : public Texture
	{
	public:
		[[nodiscard]] static Ref<Texture2D> Create(TextureFormat format);
		[[nodiscard]] static Ref<Texture2D> Create(uint32_t width, uint32_t height, TextureFormat format);
		[[nodiscard]] static Ref<Texture2D> Create(const std::string& path, TextureFormat format);
	};

	class TextureCubemap : public Texture
	{
	public:
		[[nodiscard]] static Ref<TextureCubemap> Create();
		[[nodiscard]] static Ref<TextureCubemap> Create(const std::string& path);

		virtual void BindIrradianceMap(uint32_t slot) const = 0;
		virtual void BindRadianceMap(uint32_t slot) const = 0;

		[[nodiscard]] virtual uint64_t GetHRDRendererID() const = 0;
	};
}
