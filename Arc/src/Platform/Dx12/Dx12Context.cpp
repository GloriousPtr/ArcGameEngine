#include "arcpch.h"
#include "Dx12Context.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comutil.h>
#include <wrl.h>
#include "d3dx12.h"

#ifdef ARC_DEBUG
#include <dxgidebug.h>
#endif

#include <backends/imgui_impl_dx12.h>

#include "DxHelper.h"
#include "Dx12Resources.h"

namespace ArcEngine
{
	static const char* GetVendorName(UINT vendorId)
	{
		if (vendorId == 0x1002)
			return "Advanced Micro Devices, Inc. (AMD)";
		if (vendorId == 0x10DE || vendorId == 0x163C)
			return "NVIDIA Corporation";
		if (vendorId == 0x8086)
			return "Intel Corporation";

		return "Unknown Vendor";
	}

	template<typename Adapter, typename Device>
	static bool GetHardwareAdapter(auto* pFactory, Adapter** ppAdapter, Device** ppDevice, D3D_FEATURE_LEVEL minFeatureLevel)
	{
		using namespace Microsoft::WRL;

		*ppAdapter = nullptr;
		*ppDevice = nullptr;
		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			Adapter* adapter = nullptr;
			if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)))
				break;

			Device* device;
			if (SUCCEEDED(D3D12CreateDevice(adapter, minFeatureLevel, IID_PPV_ARGS(&device))))
			{
				*ppAdapter = adapter;
				*ppDevice = device;
				return true;
			}

			adapter->Release();
			device->Release();
		}
		return false;
	}

	struct Dx12Fence
	{
		ID3D12Fence* Fence = nullptr;
		uint64_t Value{};
		HANDLE Event{};
	};

	struct Dx12Frame
	{
		Dx12Fence Fence{};
		ID3D12Resource* RtvBuffer = nullptr;
		DescriptorHandle RtvHandle{};

		ID3D12CommandAllocator* GraphicsCommandAllocator = nullptr;
		ID3D12GraphicsCommandList6* GraphicsCommandList = nullptr;

		ID3D12CommandAllocator* UploadCommandAllocator = nullptr;
		ID3D12GraphicsCommandList6* UploadCommandList = nullptr;

		std::vector<IUnknown**> DeferredReleases{};
		bool DeferedReleasesFlag = false;
		bool DeferedReleasesFlagHandles = false;

		inline static uint32_t CurrentBackBuffer = 0;

		void ResetGraphicsCommands() const
		{
			GraphicsCommandAllocator->Reset();
			GraphicsCommandList->Reset(GraphicsCommandAllocator, nullptr);
		}

		void CloseGraphicsCommands() const
		{
			GraphicsCommandList->Close();
		}

		void ResetUploadCommands() const
		{
			UploadCommandAllocator->Reset();
			UploadCommandList->Reset(UploadCommandAllocator, nullptr);
		}

		void CloseUploadCommands() const
		{
			UploadCommandList->Close();
		}
	};

	static void DebugMessageCallback(
		[[maybe_unused]] D3D12_MESSAGE_CATEGORY category,
		[[maybe_unused]] D3D12_MESSAGE_SEVERITY severity,
		[[maybe_unused]] D3D12_MESSAGE_ID id,
		[[maybe_unused]] LPCSTR pDescription,
		[[maybe_unused]] void* pContext)
	{
		OPTICK_EVENT();

		switch (severity)
		{
			case D3D12_MESSAGE_SEVERITY_CORRUPTION:		ARC_CORE_CRITICAL(pDescription); return;
			case D3D12_MESSAGE_SEVERITY_ERROR:			ARC_CORE_ERROR(pDescription); return;
			case D3D12_MESSAGE_SEVERITY_WARNING:		ARC_CORE_WARN(pDescription); return;
		}
	}

	static void WaitForFence(ID3D12Fence* fence, uint64_t completionValue, HANDLE waitEvent)
	{
		ARC_PROFILE_SCOPE()

		if (fence->GetCompletedValue() < completionValue)
		{
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	inline static IDXGIFactory7* s_Factory;
	inline static ID3D12Device8* s_Device;
	inline static ID3D12CommandQueue* s_CommandQueue;
	inline static IDXGISwapChain4* s_Swapchain;
	inline static Dx12Frame s_Frames[Dx12Context::FrameCount];

	inline static std::mutex s_DeferredReleasesMutex{};
	inline static DescriptorHeap s_RtvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
	inline static DescriptorHeap s_DsvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
	inline static DescriptorHeap s_SrvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
	inline static DescriptorHeap s_UavDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

	inline static uint32_t s_Width = 0;
	inline static uint32_t s_Height = 0;

#ifdef ARC_DEBUG
#define ENABLE_DX12_DEBUG_MESSAGES
	static DWORD s_DebugCallbackCookie;
#endif

	Dx12Context::Dx12Context(HWND hwnd)
		: m_Hwnd(hwnd)
	{
		ARC_PROFILE_SCOPE()

		ARC_CORE_ASSERT(hwnd, "Handle is null!")

		RECT clientRect;
		GetClientRect(m_Hwnd, &clientRect);
		s_Width = clientRect.right - clientRect.left;
		s_Height = clientRect.bottom - clientRect.top;
	}

	Dx12Context::~Dx12Context()
	{
		ARC_PROFILE_SCOPE()

		using namespace Microsoft::WRL;

		for (auto& frame : s_Frames)
		{
			s_RtvDescHeap.Free(frame.RtvHandle);
			frame.RtvBuffer->Release();
			frame.Fence.Fence->Release();
			frame.GraphicsCommandList->Release();
			frame.GraphicsCommandAllocator->Release();
			frame.UploadCommandList->Release();
			frame.UploadCommandAllocator->Release();
		}

		for (uint32_t i = 0; i < FrameCount; ++i)
			ProcessDeferredReleases(i);

		s_RtvDescHeap.Release();
		s_DsvDescHeap.Release();
		s_SrvDescHeap.Release();
		s_UavDescHeap.Release();

		for (uint32_t i = 0; i < FrameCount; ++i)
			ProcessDeferredReleases(i);

		s_Swapchain->Release();
		s_CommandQueue->Release();

#ifdef ARC_DEBUG
#ifdef ENABLE_DX12_DEBUG_MESSAGES
		ID3D12InfoQueue1* infoQueue1 = nullptr;
		ID3D12InfoQueue* infoQueue = nullptr;
		if (FAILED(s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue1))))
		{
			if (FAILED(s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
				infoQueue = nullptr;

			infoQueue1 = nullptr;
		}
#else
		ID3D12InfoQueue* infoQueue;
		if (FAILED(s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
			infoQueue = nullptr;
#endif // ENABLE_DX12_DEBUG_MESSAGES

		ComPtr<ID3D12DebugDevice2> debugDevice;
		s_Device->QueryInterface(IID_PPV_ARGS(&debugDevice));
#endif // ILLUMINO_DEBUG

		s_Device->Release();
		s_Factory->Release();

#ifdef ARC_DEBUG
		if (infoQueue1)
		{
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			infoQueue1->UnregisterMessageCallback(s_DebugCallbackCookie);
			infoQueue1->Release();
		}
		else if (infoQueue)
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			infoQueue->Release();
		}
		
		if (debugDevice)
		{
			debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		}
#endif
	}

	void Dx12Context::Init()
	{
		ARC_PROFILE_SCOPE()

		using namespace Microsoft::WRL;

		UINT dxgiFactoryFlags = 0;

#ifdef ARC_DEBUG
		{
			ComPtr<ID3D12Debug3> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
				debugController->EnableDebugLayer();
			else
				ARC_CORE_WARN("Failed to enable DirectX 12 debug layer. Make sure Graphics Tools (Apps & features/Optional features) is installed for it to work!");
		}

		{
			ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
			DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue));
			dxgiInfoQueue->GetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING);
			dxgiInfoQueue->GetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR);
			dxgiInfoQueue->GetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION);
		}

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&s_Factory)), "Failed to create DXGI Factory")

		const D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
		IDXGIAdapter4* adapter;
		GetHardwareAdapter(s_Factory, &adapter, &s_Device, minFeatureLevel);

		ARC_CORE_ASSERT(adapter, "Failed to adapter");
		ARC_CORE_ASSERT(s_Device, "Failed to create device");
		s_Device->SetName(L"Main D3D12 Device");
		{
			// Logging info
			DXGI_ADAPTER_DESC3 adapterDesc;
			adapter->GetDesc3(&adapterDesc);

			ARC_CORE_INFO("DirectX Info:");
			_bstr_t wcDesc(adapterDesc.Description);
			const char* desc = wcDesc;
			ARC_CORE_INFO("  Vendor: {}", GetVendorName(adapterDesc.VendorId));
			ARC_CORE_INFO("  Renderer: {}", desc);
		}
		// Adapter no longer needed
		adapter->Release();

#ifdef ARC_DEBUG
#ifdef ENABLE_DX12_DEBUG_MESSAGES
		{
			ComPtr<ID3D12InfoQueue1> infoQueue1;
			if (SUCCEEDED(s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue1))))
			{
				infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
				infoQueue1->RegisterMessageCallback(DebugMessageCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &s_DebugCallbackCookie);
			}
			else
			{
				ARC_CORE_WARN("Could not enable enable DX12 debug messages on console window!");
				ComPtr<ID3D12InfoQueue> infoQueue;
				if (SUCCEEDED(s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
				{
					infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
					infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
					infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
				}
				else
				{
					ARC_CORE_ERROR("Could not enable debugging support for the device!");
				}
			}
		}
#else
		{
			ARC_CORE_WARN("Support for DX12 debug messages on console window is disabled, define ENABLE_DX12_DEBUG_MESSAGES to enable the support, it requires Windows 11 SDK!");
			ComPtr<ID3D12InfoQueue> infoQueue;
			HRESULT result = s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
			if (SUCCEEDED(result))
			{
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			}
			else
			{
				ARC_CORE_ERROR("Could not enable debugging support for the DX12 device!");
			}
		}
#endif // ENABLE_DX12_DEBUG_MESSAGES
#endif // ARC_CORE_DEBUG

		bool heapInitResult = true;
		heapInitResult &= s_RtvDescHeap.Init(512, false);
		heapInitResult &= s_DsvDescHeap.Init(512, false);
		heapInitResult &= s_SrvDescHeap.Init(4096, true);
		heapInitResult &= s_UavDescHeap.Init(512, false);
		ARC_CORE_ASSERT(heapInitResult)

		s_RtvDescHeap.Heap()->SetName(L"RTV Descriptor Heap");
		s_DsvDescHeap.Heap()->SetName(L"DSV Descriptor Heap");
		s_SrvDescHeap.Heap()->SetName(L"SRV Descriptor Heap");
		s_UavDescHeap.Heap()->SetName(L"UAV Descriptor Heap");

		// Create command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 0;
		ThrowIfFailed(s_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&s_CommandQueue)), "Failed to create command queue");
		s_CommandQueue->SetName(L"Main D3D12 Command Queue");

		// Create Fences
		for (auto& frame : s_Frames)
		{
			frame.Fence.Event = CreateEvent(nullptr, false, false, nullptr);
			frame.Fence.Value = 0;
			s_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&(frame.Fence.Fence)));
		}

		CreateSwapchain();

		// Create allocators and command lists
		int tempInt = 0;
		for (auto& frame : s_Frames)
		{
			s_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&(frame.GraphicsCommandAllocator)));
			s_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.GraphicsCommandAllocator, nullptr, IID_PPV_ARGS(&(frame.GraphicsCommandList)));

			s_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&(frame.UploadCommandAllocator)));
			s_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.UploadCommandAllocator, nullptr, IID_PPV_ARGS(&(frame.UploadCommandList)));

			std::string cmdAllocatorName = fmt::format("Graphics Command Allocator {}", tempInt);
			_bstr_t wCmdAllocatorName(cmdAllocatorName.c_str());
			frame.GraphicsCommandAllocator->SetName(wCmdAllocatorName);
			std::string cmdListName = fmt::format("Graphics Command List {}", tempInt);
			_bstr_t wCmdListName(cmdListName.c_str());
			frame.GraphicsCommandList->SetName(wCmdListName);

			frame.GraphicsCommandList->Close();
			frame.UploadCommandList->Close();

			++tempInt;
		}

		s_Frames[Dx12Frame::CurrentBackBuffer].ResetUploadCommands();
	}

	void Dx12Context::SwapBuffers()
	{
		ARC_PROFILE_SCOPE()

		auto& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];

		backFrame.CloseUploadCommands();



		constexpr uint32_t numCommandList = 2;
		const std::array<ID3D12CommandList*, numCommandList> commandLists
		{
			backFrame.GraphicsCommandList,
			backFrame.UploadCommandList
		};
		s_CommandQueue->ExecuteCommandLists(numCommandList, commandLists.data());

		s_Swapchain->Present(m_SyncInterval, m_PresentFlags);

		const auto fenceValue = backFrame.Fence.Value;
		s_CommandQueue->Signal(backFrame.Fence.Fence, fenceValue);
		++backFrame.Fence.Value;

		WaitForFence(backFrame.Fence.Fence, fenceValue, backFrame.Fence.Event);

		if (m_ShouldResize)
		{
			for (auto& frame : s_Frames)
			{
				s_RtvDescHeap.Free(frame.RtvHandle);
				frame.RtvBuffer->Release();
				frame.RtvBuffer = nullptr;
			}

			DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
			s_Swapchain->GetDesc1(&swapchainDesc);
			s_Swapchain->ResizeBuffers(swapchainDesc.BufferCount, s_Width, s_Height, swapchainDesc.Format, swapchainDesc.Flags);
			CreateRTV();
			m_ShouldResize = false;
		}

		Dx12Frame::CurrentBackBuffer = s_Swapchain->GetCurrentBackBufferIndex();






		s_Frames[Dx12Frame::CurrentBackBuffer].ResetUploadCommands();
	}

	void Dx12Context::SetSyncInterval(uint32_t value)
	{
		m_SyncInterval = value;
		m_PresentFlags |= m_SyncInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0;
	}

	ID3D12Device8* Dx12Context::GetDevice()
	{
		return s_Device;
	}

	ID3D12GraphicsCommandList6* Dx12Context::GetGraphicsCommandList()
	{
		return s_Frames[Dx12Frame::CurrentBackBuffer].GraphicsCommandList;
	}

	ID3D12GraphicsCommandList6* Dx12Context::GetUploadCommandList()
	{
		return s_Frames[Dx12Frame::CurrentBackBuffer].UploadCommandList;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Dx12Context::GetRtv()
	{
		return s_Frames[Dx12Frame::CurrentBackBuffer].RtvHandle.CPU;
	}

	DescriptorHeap* Dx12Context::GetSrvHeap()
	{
		return &s_SrvDescHeap;
	}

	DescriptorHeap* Dx12Context::GetRtvHeap()
	{
		return &s_RtvDescHeap;
	}

	DescriptorHeap* Dx12Context::GetDsvHeap()
	{
		return &s_DsvDescHeap;
	}

	uint32_t Dx12Context::GetWidth()
	{
		return s_Width;
	}

	uint32_t Dx12Context::GetHeight()
	{
		return s_Height;
	}

	int Dx12Context::GetSwapChainFormat()
	{
		DXGI_SWAP_CHAIN_DESC1 desc;
		s_Swapchain->GetDesc1(&desc);
		return desc.Format;
	}

	uint32_t Dx12Context::GetCurrentFrameIndex()
	{
		return Dx12Frame::CurrentBackBuffer;
	}

	void Dx12Context::SetDeferredReleaseFlag()
	{
		s_Frames[Dx12Frame::CurrentBackBuffer].DeferedReleasesFlagHandles = true;
	}

	void Dx12Context::OnBeginFrame() const
	{
		const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(s_Width), static_cast<float>(s_Height), 0.0f, 1.0f };
		const D3D12_RECT scissorRect = { 0, 0, static_cast<long>(s_Width), static_cast<long>(s_Height) };

		const auto& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];
		auto* commandList = backFrame.GraphicsCommandList;

		backFrame.ResetGraphicsCommands();

		if (backFrame.DeferedReleasesFlag || backFrame.DeferedReleasesFlagHandles)
		{
			ProcessDeferredReleases(Dx12Frame::CurrentBackBuffer);
		}

		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backFrame.RtvBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		commandList->OMSetRenderTargets(1, &(backFrame.RtvHandle.CPU), false, nullptr);

		ID3D12DescriptorHeap* srv = s_SrvDescHeap.Heap();
		commandList->SetDescriptorHeaps(1, &srv);
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
	}

	void Dx12Context::OnEndFrame() const
	{
		const auto& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];

		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backFrame.RtvBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		backFrame.GraphicsCommandList->ResourceBarrier(1, &barrier);
		backFrame.CloseGraphicsCommands();
	}

	void Dx12Context::CreateRTV() const
	{
		// Create RTV
		int tempInt = 0;
		for (auto& frame : s_Frames)
		{
			s_Swapchain->GetBuffer(tempInt, IID_PPV_ARGS(&(frame.RtvBuffer)));
			frame.RtvHandle = s_RtvDescHeap.Allocate();
			s_Device->CreateRenderTargetView(frame.RtvBuffer, nullptr, frame.RtvHandle.CPU);

			std::string rtvFrameName = fmt::format("RTV Frame {}", tempInt);
			_bstr_t wRtvFrameName(rtvFrameName.c_str());
			frame.RtvBuffer->SetName(wRtvFrameName);

			++tempInt;
		}
	}

	void Dx12Context::CreateSwapchain() const
	{
		// Create Swapchain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = s_Width;
		swapChainDesc.Height = s_Height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		IDXGISwapChain1* newSwapchain;
		s_Factory->CreateSwapChainForHwnd(s_CommandQueue, m_Hwnd, &swapChainDesc, nullptr, nullptr, &newSwapchain);
		s_Swapchain = reinterpret_cast<IDXGISwapChain4*>(newSwapchain);
		
		CreateRTV();

		Dx12Frame::CurrentBackBuffer = s_Swapchain->GetCurrentBackBufferIndex();
	}

	void Dx12Context::ResizeSwapchain(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return;

		m_ShouldResize = true;
		s_Width = width;
		s_Height = height;
	}

	void Dx12Context::ProcessDeferredReleases(uint32_t frameIndex)
	{
		std::lock_guard lock(s_DeferredReleasesMutex);

		Dx12Frame& backFrame = s_Frames[frameIndex];

		if (backFrame.DeferedReleasesFlagHandles)
		{
			backFrame.DeferedReleasesFlagHandles = false;
			s_RtvDescHeap.ProcessDeferredFree(frameIndex);
			s_DsvDescHeap.ProcessDeferredFree(frameIndex);
			s_SrvDescHeap.ProcessDeferredFree(frameIndex);
			s_UavDescHeap.ProcessDeferredFree(frameIndex);
		}

		if (backFrame.DeferedReleasesFlag)
		{
			backFrame.DeferedReleasesFlag = false;
			auto& resources = backFrame.DeferredReleases;
			for (auto& resource : resources)
			{
				(*resource)->Release();
				*resource = nullptr;
			}
			resources.clear();
		}
	}

	void Dx12Context::DeferredRelease(IUnknown** resource)
	{
		Dx12Frame& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];
		std::lock_guard lock(s_DeferredReleasesMutex);
		backFrame.DeferredReleases.push_back(resource);
		backFrame.DeferedReleasesFlag = true;
	}
}
