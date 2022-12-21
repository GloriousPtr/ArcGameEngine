#pragma once

namespace ArcEngine
{
	using TextureData = void*;

	class Texture
	{
	public:
		virtual ~Texture() = default;
		
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint64_t GetRendererID() const = 0;
		virtual const std::string& GetPath() const = 0;

		virtual void SetData(TextureData data, uint32_t size) = 0;
		virtual void Invalidate(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels) = 0;
		
		virtual void Bind(uint32_t slot = 0) const = 0;

		bool operator==(const Texture& other) const { return GetRendererID() == other.GetRendererID(); }
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create();
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);
	};

	class TextureCubemap : public Texture
	{
	public:
		static Ref<TextureCubemap> Create();
		static Ref<TextureCubemap> Create(const std::string& path);

		virtual void BindIrradianceMap(uint32_t slot) const = 0;
		virtual void BindRadianceMap(uint32_t slot) const = 0;

		virtual uint64_t GetHRDRendererID() const = 0;
	};
}
