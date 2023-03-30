#pragma once

#include "Arc/Renderer/Texture.h"
#include "Platform/Dx12/Dx12Resources.h"

namespace D3D12MA
{
	class Allocation;
}

namespace ArcEngine
{
	class Dx12Texture2D : public Texture2D
	{
	public:
		Dx12Texture2D(uint32_t width, uint32_t height, TextureFormat format);
		Dx12Texture2D(const std::string& path, TextureFormat format);
		~Dx12Texture2D() override;

		Dx12Texture2D(const Dx12Texture2D& other) = default;
		Dx12Texture2D(Dx12Texture2D&& other) = default;

		[[nodiscard]] uint32_t GetWidth() const override { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const override { return m_Height; }
		[[nodiscard]] TextureFormat GetTextureFormat() const override { return m_Format; }
		[[nodiscard]] uint64_t GetRendererID() const override { return m_Handle.GPU.ptr; }
		[[nodiscard]] uint32_t GetIndex() const override;
		[[nodiscard]] const std::string& GetPath() const override { return m_Path; }

		void SetData(const TextureData data, [[maybe_unused]] uint32_t size) override;

		void Bind(uint32_t slot = 0) const override;

	private:
		TextureFormat						m_Format = TextureFormat::None;
		uint32_t							m_Width = 0;
		uint32_t							m_Height = 0;
		uint32_t							m_Channels = 0;
		std::string							m_Path{};
		DescriptorHandle					m_Handle{};
		D3D12_GPU_DESCRIPTOR_HANDLE			m_HeapStart{};
		D3D12MA::Allocation*				m_ImageAllocation = nullptr;
		D3D12MA::Allocation*				m_UploadImageAllocation = nullptr;
	};

	class Dx12TextureCube : public TextureCube
	{
	public:
		Dx12TextureCube(const std::string& path, TextureFormat format);
		~Dx12TextureCube() override;

		[[nodiscard]] uint32_t GetWidth() const override { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const override { return m_Height; }
		[[nodiscard]] TextureFormat GetTextureFormat() const override { return m_Format; }
		[[nodiscard]] uint64_t GetRendererID() const override { return m_SrvHandle.GPU.ptr; }
		[[nodiscard]] uint32_t GetIndex() const override;
		[[nodiscard]] const std::string& GetPath() const override { return m_Path; }

		void SetData(const TextureData data, [[maybe_unused]] uint32_t size) override;

		void Bind(uint32_t slot = 0) const override;

		void BindIrradianceMap([[maybe_unused]] uint32_t slot) const override {}
		void BindRadianceMap([[maybe_unused]] uint32_t slot) const override {}
		[[nodiscard]] uint64_t GetHRDRendererID() const override { return m_HDRHandle.GPU.ptr; }

	private:
		TextureFormat						m_Format = TextureFormat::None;
		uint32_t							m_Width = 0;
		uint32_t							m_Height = 0;
		std::string							m_Path;
		DescriptorHandle					m_HDRHandle{};
		DescriptorHandle					m_SrvHandle{};
		DescriptorHandle					m_UavHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE			m_HeapStart{};

		D3D12MA::Allocation*				m_HDRImageAllocation = nullptr;
		D3D12MA::Allocation*				m_HDRUploadImageAllocation = nullptr;
		D3D12MA::Allocation*				m_ImageAllocation = nullptr;
		D3D12MA::Allocation*				m_UploadImageAllocation = nullptr;
	};
}
