#include "arcpch.h"
#include "Dx12Texture.h"

#include "d3dx12.h"
#include "Dx12Allocator.h"

#include <stb_image.h>

#include "DxHelper.h"
#include "Arc/Renderer/PipelineState.h"
#include "Arc/Renderer/Renderer.h"

namespace ArcEngine
{
	Dx12Texture2D::Dx12Texture2D(uint32_t width, uint32_t height, TextureFormat format)
		: m_Format(format), m_Width(width), m_Height(height)
	{
		m_Channels = ChannelCountFromFormat(m_Format);
	}

	Dx12Texture2D::Dx12Texture2D(const std::string& path, TextureFormat format)
		: m_Format(format)
	{
		ARC_PROFILE_SCOPE()

		m_Channels = ChannelCountFromFormat(m_Format);

		stbi_set_flip_vertically_on_load(0);

		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			ARC_PROFILE_SCOPE("stbi_load Texture")
			data = stbi_load(path.c_str(), &width, &height, &channels, static_cast<int>(m_Channels));
		}
		ARC_CORE_ASSERT(data, "Failed to load image!")

		InvalidateImpl(path, width, height, data);

		stbi_image_free(data);
	}

	Dx12Texture2D::~Dx12Texture2D()
	{
		ARC_PROFILE_SCOPE()

		Dx12Context::GetSrvHeap()->Free(m_Handle);

		if (m_UploadImageAllocation)
			m_UploadImageAllocation->Release();
		if (m_ImageAllocation)
			m_ImageAllocation->Release();
	}

	uint32_t Dx12Texture2D::GetIndex() const
	{
		return static_cast<uint32_t>(m_Handle.GPU.ptr - m_HeapStart.ptr) / Dx12Context::GetSrvHeap()->DescriptorSize();
	}

	void Dx12Texture2D::SetData(const TextureData data, uint32_t size)
	{
		ARC_PROFILE_SCOPE()

		ARC_CORE_ASSERT(m_Width * m_Height == size / m_Channels)

		InvalidateImpl("", m_Width, m_Height, data);
	}

	void Dx12Texture2D::Invalidate(std::string_view path, uint32_t width, uint32_t height, const void* data)
	{
		ARC_PROFILE_SCOPE()

		InvalidateImpl(path, width, height, data);
	}

	void Dx12Texture2D::Bind(uint32_t slot) const
	{
		ARC_PROFILE_SCOPE()

		Dx12Context::GetGraphicsCommandList()->SetGraphicsRootDescriptorTable(slot, m_Handle.GPU);
	}

	void Dx12Texture2D::InvalidateImpl(std::string_view path, uint32_t width, uint32_t height, const void* data)
	{
		m_Width = width;
		m_Height = height;
		m_Path = path;

		m_Handle = Dx12Context::GetSrvHeap()->Allocate();
		m_HeapStart = Dx12Context::GetSrvHeap()->GpuStart();

		Dx12Utils::CreateTexture(&m_ImageAllocation, &m_UploadImageAllocation, D3D12_SRV_DIMENSION_TEXTURE2D, Dx12Utils::Dx12FormatFromTextureFormat(m_Format), m_Width, m_Height, 1, m_Channels, data, &m_Handle, nullptr);
	}
}
