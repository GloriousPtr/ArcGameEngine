#include "arcpch.h"
#include "Dx12Texture.h"

#include <stb_image.h>

#include "d3dx12.h"

namespace ArcEngine
{
	Dx12Texture2D::Dx12Texture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
	}

	Dx12Texture2D::Dx12Texture2D(const std::string& path)
	{
		ARC_PROFILE_SCOPE()

		stbi_set_flip_vertically_on_load(0);

		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			ARC_PROFILE_SCOPE("stbi_load Texture")
			data = stbi_load(path.c_str(), &width, &height, &channels, 4);
		}
		ARC_CORE_ASSERT(data, "Failed to load image!")

		InvalidateImpl(path, width, height, data, channels);

		stbi_image_free(data);
	}

	Dx12Texture2D::~Dx12Texture2D()
	{
		ARC_PROFILE_SCOPE()

		Dx12Context::GetSrvHeap()->Free(m_Handle);

		if (m_UploadImage)
			m_UploadImage->Release();
		if(m_Image)
			m_Image->Release();
	}

	uint32_t Dx12Texture2D::GetIndex() const
	{
		return static_cast<uint32_t>(m_Handle.GPU.ptr - m_HeapStart.ptr) / Dx12Context::GetSrvHeap()->DescriptorSize();
	}

	void Dx12Texture2D::SetData(const TextureData data, uint32_t size)
	{
		ARC_PROFILE_SCOPE()

		ARC_CORE_ASSERT(m_Width * m_Height == size / 4)

		InvalidateImpl("", m_Width, m_Height, data, 4);
	}

	void Dx12Texture2D::Invalidate(std::string_view path, uint32_t width, uint32_t height, const void* data,
		uint32_t channels)
	{
		InvalidateImpl(path, width, height, data, channels);
	}

	void Dx12Texture2D::Bind(uint32_t slot) const
	{
		ARC_PROFILE_SCOPE()

		Dx12Context::GetGraphicsCommandList()->SetGraphicsRootDescriptorTable(slot, m_Handle.GPU);
	}

	void Dx12Texture2D::InvalidateImpl(std::string_view path, uint32_t width, uint32_t height, const void* data,
		uint32_t channels)
	{
		ARC_PROFILE_SCOPE()

		m_Width = width;
		m_Height = height;

		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		switch (channels)
		{
			case 1:
				format = DXGI_FORMAT_R8_UNORM;
				break;
			case 2:
				format = DXGI_FORMAT_R8G8_UNORM;
				break;
			case 3:
				format = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			case 4:
				format = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			default:
				ARC_CORE_ERROR("Texture channel count is not within (1-4) range. Channel count: {}", channels);
				break;
		}

		const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1);
		Dx12Context::GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Image));

		const auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto uploadBufferSize = GetRequiredIntermediateSize(m_Image, 0, 1);
		const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		Dx12Context::GetDevice()->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_UploadImage));

		D3D12_SUBRESOURCE_DATA srcData{};
		srcData.pData = data;
		srcData.RowPitch = width * 4;
		srcData.SlicePitch = width * height * 4;

		auto* commandList = Dx12Context::GetUploadCommandList();
		UpdateSubresources(commandList, m_Image, m_UploadImage, 0, 0, 1, &srcData);
		const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_Image, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = format;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		m_Handle = Dx12Context::GetSrvHeap()->Allocate();
		m_HeapStart = Dx12Context::GetSrvHeap()->GpuStart();
		Dx12Context::GetDevice()->CreateShaderResourceView(m_Image, &srvDesc, m_Handle.CPU);
	}
}
