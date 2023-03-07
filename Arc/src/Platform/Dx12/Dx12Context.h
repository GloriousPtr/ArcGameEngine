#pragma once

#include "Arc/Renderer/GraphicsContext.h"

class IUnknown;
class ID3D12Device10;
class ID3D12CommandAllocator;
class ID3D12GraphicsCommandList7;
class ID3D12DescriptorHeap;

namespace ArcEngine
{
	class DescriptorHeap;

	class Dx12Context : public GraphicsContext
	{
	public:
		static constexpr size_t FrameCount = 3;

	public:
		explicit Dx12Context(HWND hwnd);
		~Dx12Context() override;

		void Init() override;
		void SwapBuffers() override;
		void SetSyncInterval(uint32_t value) override;

		void OnBeginFrame() const;
		void OnEndFrame() const;

		static ID3D12Device10* GetDevice();
		static ID3D12CommandAllocator* GetCommandAllocator();
		static ID3D12GraphicsCommandList7* GetGraphicsCommandList();
		static int GetSwapChainFormat();
		static uint32_t GetCurrentFrameIndex();
		static void SetDeferredReleaseFlag();

		static DescriptorHeap* GetSrvHeap();
		
	private:
		void CreateSwapchain() const;
		void ResizeSwapchain(uint32_t width, uint32_t height);

		static void ProcessDeferredReleases(uint32_t frameIndex);
		static void DeferredRelease(IUnknown* resource);

	private:
		HWND m_Hwnd;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		uint32_t m_SyncInterval = 1;
		uint32_t m_PresentFlags = 0;
		bool m_ShouldResize = false;

		friend class ImGuiLayer;
		friend class DescriptorHeap;
		friend class Dx12RendererAPI;
	};
}
