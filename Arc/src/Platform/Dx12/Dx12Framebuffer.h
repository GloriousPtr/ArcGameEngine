#pragma once

#include <D3D12MemAlloc.h>

#include "Arc/Renderer/Framebuffer.h"

#include "Platform/Dx12/Dx12Resources.h"

namespace D3D12MA
{
	class Allocation;
}

namespace ArcEngine
{
	class Dx12Framebuffer : public Framebuffer
	{
	public:
		explicit Dx12Framebuffer(const FramebufferSpecification& spec);
		~Dx12Framebuffer() override;

		Dx12Framebuffer(const Dx12Framebuffer& other) = default;
		Dx12Framebuffer(Dx12Framebuffer&& other) = default;

		void Invalidate();

		void Bind() override;
		void Unbind() override;
		void Clear() override;
		void BindColorAttachment(uint32_t index, uint32_t slot) override;
		void BindDepthAttachment(uint32_t slot) override;
		void Resize(uint32_t width, uint32_t height) override;
		[[nodiscard]] uint64_t GetColorAttachmentRendererID(uint32_t index) const override { return m_ColorAttachments[Dx12Context::GetCurrentFrameIndex()][index].SrvHandle.GPU.ptr; }
		[[nodiscard]] uint64_t GetDepthAttachmentRendererID() const override { return m_DepthAttachment[Dx12Context::GetCurrentFrameIndex()].DsvHandle.GPU.ptr; }
		[[nodiscard]] const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		void TransitionTo(D3D12_RESOURCE_STATES colorAttachmentState, D3D12_RESOURCE_STATES depthAttachmentState);

	private:
		FramebufferSpecification m_Specification;

		struct ColorFrame
		{
			D3D12_RESOURCE_STATES		State = D3D12_RESOURCE_STATE_RENDER_TARGET;
			D3D12MA::Allocation*		Allocation = nullptr;
			DescriptorHandle			SrvHandle{};
			DescriptorHandle			RtvHandle{};

			ColorFrame() = default;

			ColorFrame(D3D12_RESOURCE_STATES state, D3D12MA::Allocation* allocation, DescriptorHandle srvHandle, DescriptorHandle rtvHandle)
				: State(state), Allocation(allocation), SrvHandle(srvHandle), RtvHandle(rtvHandle)
			{
			}

			void Release(bool deferred)
			{
				State = D3D12_RESOURCE_STATE_COMMON;

				if (SrvHandle.IsValid())
					Dx12Context::GetSrvHeap()->Free(SrvHandle);
				if (RtvHandle.IsValid())
					Dx12Context::GetRtvHeap()->Free(RtvHandle);
				if (Allocation)
				{
					if (deferred)
					{
						Dx12Context::DeferredRelease(Allocation);
						Allocation = nullptr;
					}
					else
					{
						Allocation->Release();
						Allocation = nullptr;
					}
				}
			}
		};

		struct DepthFrame
		{
			D3D12_RESOURCE_STATES		State = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			D3D12MA::Allocation*		Allocation = nullptr;
			DescriptorHandle			SrvHandle{};
			DescriptorHandle			DsvHandle{};

			DepthFrame() = default;

			DepthFrame(D3D12_RESOURCE_STATES state, D3D12MA::Allocation* allocation, DescriptorHandle srvHandle, DescriptorHandle dsvHandle)
				: State(state), Allocation(allocation), SrvHandle(srvHandle), DsvHandle(dsvHandle)
			{
			}

			void Release(bool deferred)
			{
				State = D3D12_RESOURCE_STATE_COMMON;

				if (SrvHandle.IsValid())
					Dx12Context::GetSrvHeap()->Free(SrvHandle);
				if (DsvHandle.IsValid())
					Dx12Context::GetDsvHeap()->Free(DsvHandle);
				if (Allocation)
				{
					if (deferred)
					{
						Dx12Context::DeferredRelease(Allocation);
						Allocation = nullptr;
					}
					else
					{
						Allocation->Release();
						Allocation = nullptr;
					}
				}
			}
		};

		eastl::array<eastl::vector<ColorFrame>, Dx12Context::FrameCount>						m_ColorAttachments{};
		eastl::array<eastl::vector<D3D12_CPU_DESCRIPTOR_HANDLE>, Dx12Context::FrameCount>		m_RtvHandles;
		eastl::array<DepthFrame, Dx12Context::FrameCount>										m_DepthAttachment{};
		glm::vec4																				m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		glm::vec2																				m_ClearDepth = glm::vec2(1.0f, 0.0f);

		eastl::array<eastl::vector<ColorFrame>, Dx12Context::FrameCount>						m_ReleasedColorAttachments{};
		eastl::array<DepthFrame, Dx12Context::FrameCount>										m_ReleasedDepthAttachment{};

	public:

		[[nodiscard]] static DXGI_FORMAT GetDxgiFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case FramebufferTextureFormat::None:				return DXGI_FORMAT_UNKNOWN;
				case FramebufferTextureFormat::RGBA8:				return DXGI_FORMAT_R8G8B8A8_UNORM;
				case FramebufferTextureFormat::RGBA16F:				return DXGI_FORMAT_R16G16B16A16_FLOAT;
				case FramebufferTextureFormat::RGBA32F:				return DXGI_FORMAT_R32G32B32A32_FLOAT;
				case FramebufferTextureFormat::R11G11B10F:			return DXGI_FORMAT_R11G11B10_FLOAT;
				case FramebufferTextureFormat::RG16F:				return DXGI_FORMAT_R16G16_FLOAT;
				case FramebufferTextureFormat::R32I:				return DXGI_FORMAT_R32_SINT;
				case FramebufferTextureFormat::DEPTH24STENCIL8:		return DXGI_FORMAT_D32_FLOAT;

				default: ARC_CORE_ERROR("Invalid framebuffer format: {}", (int)format); return DXGI_FORMAT_UNKNOWN;
			}
		}
	};
}
