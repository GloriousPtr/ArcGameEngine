#pragma once

#include <comutil.h>
#include <Arc/Core/Base.h>

#include "d3dx12.h"
#include "Dx12Allocator.h"
#include "Arc/Renderer/Texture.h"

namespace ArcEngine::Dx12Utils
{

	#define ThrowIfFailed(fn, msg)	{\
										HRESULT hr = fn;\
										ARC_CORE_ASSERT(SUCCEEDED(hr), msg); \
									}

	#define NameResource(pResource, cstr)	{\
												_bstr_t wcstr(cstr);\
												(pResource)->SetName(wcstr);\
											}

	#define ToWCSTR(cstr) _bstr_t(cstr)
	#define ToCSTR(wcstr) static_cast<const char*>(_bstr_t(wcstr))

	static DXGI_FORMAT Dx12FormatFromTextureFormat(ArcEngine::TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::None:		return DXGI_FORMAT_UNKNOWN;
		case TextureFormat::RGBA32F:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TextureFormat::RGB32F:		return DXGI_FORMAT_R32G32B32_FLOAT;
		case TextureFormat::RG32F:		return DXGI_FORMAT_R32G32_FLOAT;
		case TextureFormat::R32F:		return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::RGBA16F:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TextureFormat::RG16F: 		return DXGI_FORMAT_R16G16_FLOAT;
		case TextureFormat::R16F: 		return DXGI_FORMAT_R16_FLOAT;
		case TextureFormat::RGBA8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RG8:		return DXGI_FORMAT_R8G8_UNORM;
		case TextureFormat::R8:			return DXGI_FORMAT_R8_UNORM;
		default:						return DXGI_FORMAT_UNKNOWN;
		}
	}

	static void CreateTexture(D3D12MA::Allocation** imageAllocation, D3D12MA::Allocation** uploadImageAllocation, D3D12_SRV_DIMENSION dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, uint32_t channels, const void* data, DescriptorHandle* srvHandle, DescriptorHandle* uavHandle)
	{
		ARC_CORE_ASSERT(imageAllocation)
			ARC_CORE_ASSERT(uploadImageAllocation)

			const auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, depth, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		Dx12Allocator::CreateTextureResource(D3D12_HEAP_TYPE_DEFAULT, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, imageAllocation);
		ID3D12Resource* resource = (*imageAllocation)->GetResource();

		D3D12MA::ALLOCATION_DESC uploadAllocation{};
		uploadAllocation.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		const auto uploadBufferSize = GetRequiredIntermediateSize(resource, 0, 1);
		const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		Dx12Allocator::CreateTextureResource(D3D12_HEAP_TYPE_UPLOAD, &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, uploadImageAllocation);
		ID3D12Resource* uploadResource = (*uploadImageAllocation)->GetResource();

		D3D12_SUBRESOURCE_DATA srcData{};
		srcData.pData = data;
		srcData.RowPitch = width * channels;
		srcData.SlicePitch = width * height * channels;

		auto* commandList = Dx12Context::GetUploadCommandList();
		UpdateSubresources(commandList, resource, uploadResource, 0, 0, 1, &srcData);
		const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);

		if (srvHandle)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = format;
			srvDesc.ViewDimension = dimension;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			switch (dimension)
			{
			case D3D12_SRV_DIMENSION_TEXTURE2D:
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				break;
			case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
				srvDesc.Texture2DArray.MipLevels = 1;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.FirstArraySlice = 0;
				srvDesc.Texture2DArray.ArraySize = depth;
				break;
			case D3D12_SRV_DIMENSION_TEXTURECUBE:
				srvDesc.TextureCube.MipLevels = 1;
				srvDesc.TextureCube.MostDetailedMip = 0;
				break;
			default:
				ARC_CORE_ASSERT(false, "Unknown texture dimensions!")
			}

			Dx12Context::GetDevice()->CreateShaderResourceView(resource, &srvDesc, srvHandle->CPU);
		}
		if (uavHandle)
		{

		}
	}
}
