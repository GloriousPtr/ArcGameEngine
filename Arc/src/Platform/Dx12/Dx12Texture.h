#pragma once

#include "Arc/Renderer/Texture.h"
#include "Platform/Dx12/Dx12Resources.h"

namespace ArcEngine
{
	class Dx12Texture2D : public Texture2D
	{
	public:
		Dx12Texture2D() = default;
		Dx12Texture2D(uint32_t width, uint32_t height);
		explicit Dx12Texture2D(const std::string& path);
		~Dx12Texture2D() override;

		Dx12Texture2D(const Dx12Texture2D& other) = default;
		Dx12Texture2D(Dx12Texture2D&& other) = default;

		[[nodiscard]] uint32_t GetWidth() const override { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const override { return m_Height; }
		[[nodiscard]] uint64_t GetRendererID() const override { return m_Handle.GPU.ptr; }
		[[nodiscard]] const std::string& GetPath() const override { return m_Path; }

		void SetData(void* data, [[maybe_unused]] uint32_t size) override;
		void Invalidate(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels) override;

		void Bind(uint32_t slot = 0) const override;

	private:
		void InvalidateImpl(std::string_view path, uint32_t width, uint32_t height, const void* data, uint32_t channels);

	private:
		std::string m_Path;
		uint32_t m_Width = 0, m_Height = 0;
		DescriptorHandle m_Handle;
		ID3D12Resource* m_Image;
		ID3D12Resource* m_UploadImage;
	};
}
