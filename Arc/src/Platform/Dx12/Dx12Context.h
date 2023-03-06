#pragma once

#include "Arc/Renderer/GraphicsContext.h"

class ID3D12DescriptorHeap;

namespace ArcEngine
{
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

	private:
		void CreateSwapchain(ID3D12DescriptorHeap* rtvHeap) const;
		void ResizeSwapchain(uint32_t width, uint32_t height);

	private:
		HWND m_Hwnd;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		uint32_t m_SyncInterval = 1;
		uint32_t m_PresentFlags = 0;

		friend class Dx12RendererAPI;
	};
}
