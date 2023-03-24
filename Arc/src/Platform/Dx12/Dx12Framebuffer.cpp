#include "arcpch.h"
#include "Dx12Framebuffer.h"

#include "DxHelper.h"
#include "d3dx12.h"
#include "Dx12Allocator.h"

#include <glm/gtc/type_ptr.hpp>

namespace ArcEngine
{
	Dx12Framebuffer::Dx12Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		ARC_PROFILE_SCOPE()

		Invalidate();
	}

	Dx12Framebuffer::~Dx12Framebuffer()
	{
		ARC_PROFILE_SCOPE()

		for (auto& depthAttachment : m_DepthAttachment)
			depthAttachment.Release(false);

		for (auto& colorAttachments : m_ColorAttachments)
		{
			for (auto& attachment : colorAttachments)
				attachment.Release(false);
		}
	}

	void Dx12Framebuffer::Invalidate()
	{
		ARC_PROFILE_SCOPE()

		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		auto* device = Dx12Context::GetDevice();
		auto* srvDescriptorHeap = Dx12Context::GetSrvHeap();
		auto* rtvDescriptorHeap = Dx12Context::GetRtvHeap();
		auto* dsvDescriptorHeap = Dx12Context::GetDsvHeap();

		auto& attachments = m_Specification.Attachments.Attachments;
		for (auto& attachment : attachments)
		{
			DXGI_FORMAT format = GetDxgiFormat(attachment.TextureFormat);

			if (attachment.TextureFormat == FramebufferTextureFormat::DEPTH24STENCIL8)
			{
				constexpr auto state = D3D12_RESOURCE_STATE_DEPTH_WRITE;

				D3D12_CLEAR_VALUE defaultDepthClear{};
				defaultDepthClear.Format = format;
				defaultDepthClear.DepthStencil.Depth = m_ClearDepth.r;
				defaultDepthClear.DepthStencil.Stencil = static_cast<uint8_t>(m_ClearDepth.g);

				D3D12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, m_Specification.Width, m_Specification.Height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
				dsvDesc.Format = format;
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Texture2D.MipSlice = 0;
				dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

				int i = 0;
				for (auto& depthAttachment : m_DepthAttachment)
				{
					D3D12MA::Allocation* allocation;
					DescriptorHandle srvHandle = srvDescriptorHeap->Allocate();
					DescriptorHandle dsvHandle = dsvDescriptorHeap->Allocate();

					Dx12Allocator::CreateRtvResource(D3D12_HEAP_TYPE_DEFAULT, &depthDesc, state, &defaultDepthClear, &allocation);
					ID3D12Resource* resource = allocation->GetResource();
					device->CreateShaderResourceView(resource, &srvDesc, srvHandle.CPU);
					device->CreateDepthStencilView(resource, &dsvDesc, dsvHandle.CPU);

					depthAttachment = { state, allocation, srvHandle, dsvHandle };

					std::string resourceName = fmt::format("Depth Resource ({}): {}", i, m_Specification.Name);
					NameResource(allocation, resourceName.c_str())
					++i;
				}
			}
			else
			{
				constexpr auto state = D3D12_RESOURCE_STATE_RENDER_TARGET;

				D3D12_CLEAR_VALUE defaultColorClear{};
				defaultColorClear.Format = format;
				memcpy(&defaultColorClear.Color, glm::value_ptr(m_ClearColor), sizeof(glm::vec4));

				D3D12_RESOURCE_DESC colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, m_Specification.Width, m_Specification.Height, 1, 1, m_Specification.Samples, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

				srvDesc.Format = format;
				rtvDesc.Format = format;

				int i = 0;
				for (auto& colorAttachment : m_ColorAttachments)
				{
					D3D12MA::Allocation* allocation;
					DescriptorHandle srvHandle = srvDescriptorHeap->Allocate();
					DescriptorHandle rtvHandle = rtvDescriptorHeap->Allocate();

					Dx12Allocator::CreateRtvResource(D3D12_HEAP_TYPE_DEFAULT, &colorDesc, state, &defaultColorClear, &allocation);
					ID3D12Resource* resource = allocation->GetResource();
					device->CreateShaderResourceView(resource, &srvDesc, srvHandle.CPU);
					device->CreateRenderTargetView(resource, &rtvDesc, rtvHandle.CPU);

					colorAttachment.emplace_back(state, allocation, srvHandle, rtvHandle);
					m_RtvHandles[i].emplace_back(rtvHandle.CPU);

					std::string resourceName = fmt::format("Color Resource ({}): {}", i, m_Specification.Name);
					NameResource(allocation, resourceName.c_str())
					++i;
				}
			}
		}
	}

	void Dx12Framebuffer::Bind()
	{
		ARC_PROFILE_SCOPE()

		auto* commandList = Dx12Context::GetGraphicsCommandList();
		const auto backFrame = Dx12Context::GetCurrentFrameIndex();

		const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DepthAttachment[backFrame].DsvHandle.CPU;
		commandList->OMSetRenderTargets(m_RtvHandles[backFrame].size(), m_RtvHandles[backFrame].data(), true, dsvHandle.ptr != 0 ? &dsvHandle : nullptr);

		const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)m_Specification.Width, (float)m_Specification.Height, 0.0f, 1.0f };
		const D3D12_RECT scissor = { 0, 0, m_Specification.Width, m_Specification.Height };

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissor);
	}

	void Dx12Framebuffer::Unbind()
	{
		ARC_PROFILE_SCOPE()

		TransitionTo(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		auto* commandList = Dx12Context::GetGraphicsCommandList();

		const auto rtv = Dx12Context::GetRtv();
		const auto width = Dx12Context::GetWidth();
		const auto height = Dx12Context::GetHeight();

		commandList->OMSetRenderTargets(1, &rtv, true, nullptr);

		const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
		const D3D12_RECT scissorRect = { 0, 0, width, height };
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		TransitionTo(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	void Dx12Framebuffer::Clear()
	{
		ARC_PROFILE_SCOPE()

		auto* commandList = Dx12Context::GetGraphicsCommandList();
		const auto backFrame = Dx12Context::GetCurrentFrameIndex();

		const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DepthAttachment[backFrame].DsvHandle.CPU;

		const auto& rtvHandles = m_RtvHandles[backFrame];
		for (const auto& rtv : rtvHandles)
			commandList->ClearRenderTargetView(rtv, glm::value_ptr(m_ClearColor), 0, nullptr);
		if (dsvHandle.ptr != 0)
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, m_ClearDepth.r, m_ClearDepth.g, 0, nullptr);
	}

	void Dx12Framebuffer::BindColorAttachment(uint32_t index, uint32_t slot)
	{
		ARC_PROFILE_SCOPE()

		const auto backFrame = Dx12Context::GetCurrentFrameIndex();
		Dx12Context::GetGraphicsCommandList()->SetGraphicsRootDescriptorTable(slot, m_ColorAttachments[backFrame][index].SrvHandle.GPU);
	}

	void Dx12Framebuffer::BindDepthAttachment(uint32_t slot)
	{
		ARC_PROFILE_SCOPE()

		const auto backFrame = Dx12Context::GetCurrentFrameIndex();
		Dx12Context::GetGraphicsCommandList()->SetGraphicsRootDescriptorTable(slot, m_DepthAttachment[backFrame].SrvHandle.GPU);
	}

	void Dx12Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE()

		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			ARC_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		for (auto& attachments : m_ReleasedColorAttachments)
			attachments.clear();
		for (auto& attachment : m_ReleasedDepthAttachment)
			attachment = {};

		m_ReleasedColorAttachments = m_ColorAttachments;
		m_ReleasedDepthAttachment = m_DepthAttachment;
		
		for (auto& depthAttachment : m_ReleasedDepthAttachment)
			depthAttachment.Release(true);
		for (auto& attachments : m_ReleasedColorAttachments)
		{
			for (auto& attachment : attachments)
				attachment.Release(true);
		}

		for (auto& depthAttachment : m_DepthAttachment)
			depthAttachment = {};
		for (auto& colorAttachments : m_ColorAttachments)
			colorAttachments.clear();
		for (auto& rtvHandles : m_RtvHandles)
			rtvHandles.clear();
			
		Invalidate();
	}

	void Dx12Framebuffer::TransitionTo(D3D12_RESOURCE_STATES colorAttachmentState, D3D12_RESOURCE_STATES depthAttachmentState)
	{
		ARC_PROFILE_SCOPE()

		const auto backFrame = Dx12Context::GetCurrentFrameIndex();

		D3D12_RESOURCE_BARRIER barriers[20];
		int numBarriers = 0;
		auto& colorAttachments = m_ColorAttachments[backFrame];
		for (auto& attachment : colorAttachments)
		{
			barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(attachment.Allocation->GetResource(), attachment.State, colorAttachmentState);
			attachment.State = colorAttachmentState;
			++numBarriers;
		}
		if (m_DepthAttachment[backFrame].Allocation)
		{
			barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(m_DepthAttachment[backFrame].Allocation->GetResource(), m_DepthAttachment[backFrame].State, depthAttachmentState);
			m_DepthAttachment[backFrame].State = depthAttachmentState;
			++numBarriers;
		}
		Dx12Context::GetGraphicsCommandList()->ResourceBarrier(numBarriers, barriers);
	}
}
