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

	static void GetHardwareAdapter(IDXGIFactory7* pFactory, IDXGIAdapter4** ppAdapter, D3D_FEATURE_LEVEL minFeatureLevel)
	{
		*ppAdapter = nullptr;
		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			IDXGIAdapter4* pAdapter = nullptr;
			if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)))
			{
				break;
			}

			if (SUCCEEDED(D3D12CreateDevice(pAdapter, minFeatureLevel, _uuidof(ID3D12Device), nullptr)))
			{
				*ppAdapter = pAdapter;
				return;
			}
			pAdapter->Release();
		}
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
		ID3D12CommandAllocator* CommandAllocator = nullptr;
		ID3D12GraphicsCommandList7* CommandList = nullptr;

		std::vector<IUnknown**> DeferredReleases{};
		bool DeferedReleasesFlag = false;
		bool DeferedReleasesFlagHandles = false;

		inline static uint32_t CurrentBackBuffer = 0;
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
	inline static ID3D12Device10* s_Device;
	inline static ID3D12CommandQueue* s_CommandQueue;
	inline static IDXGISwapChain4* s_Swapchain;
	inline static Dx12Frame s_Frames[Dx12Context::FrameCount];

	inline static std::mutex s_DeferredReleasesMutex{};
	inline static DescriptorHeap s_RtvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
	inline static DescriptorHeap s_DsvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
	inline static DescriptorHeap s_SrvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
	inline static DescriptorHeap s_UavDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

	inline static Microsoft::WRL::ComPtr<ID3D12InfoQueue1> s_InfoQueue1 = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D12InfoQueue> s_InfoQueue = nullptr;

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
		m_Width = clientRect.right - clientRect.left;
		m_Height = clientRect.bottom - clientRect.top;
	}

	Dx12Context::~Dx12Context()
	{
		ARC_PROFILE_SCOPE()

		for (auto& frame : s_Frames)
		{
			s_RtvDescHeap.Free(frame.RtvHandle);
			frame.RtvBuffer->Release();
			frame.Fence.Fence->Release();
			frame.CommandList->Release();
			frame.CommandAllocator->Release();
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
		HRESULT hr = s_Device->QueryInterface(IID_PPV_ARGS(&s_InfoQueue1));
		
		if (SUCCEEDED(hr))
		{
			s_InfoQueue = nullptr;
		}
		else
		{
			hr = s_Device->QueryInterface(IID_PPV_ARGS(&s_InfoQueue));
			if (SUCCEEDED(hr))
			{
				s_InfoQueue1 = nullptr;
			}
		}
#else
		HRESULT hr = s_Device->QueryInterface(IID_PPV_ARGS(&s_InfoQueue));
		if (SUCCEEDED(hr))
			s_InfoQueue1 = nullptr;
#endif // ENABLE_DX12_DEBUG_MESSAGES
#endif // ILLUMINO_DEBUG

		s_Device->Release();
		s_Factory->Release();

#ifdef ARC_DEBUG
		if (s_InfoQueue1)
		{
			s_InfoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			s_InfoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			s_InfoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			s_InfoQueue1->UnregisterMessageCallback(s_DebugCallbackCookie);
			s_InfoQueue1 = nullptr;
		}
		if (s_InfoQueue)
		{
			s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			s_InfoQueue = nullptr;
		}
		IDXGIDebug1* dxgiDebug;
		DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		dxgiDebug->Release();
#endif
	}

	void Dx12Context::Init()
	{
		ARC_PROFILE_SCOPE()

		UINT dxgiFactoryFlags = 0;

#ifdef ARC_DEBUG
		ID3D12Debug6* debug;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		debug->EnableDebugLayer();
		debug->Release();

		IDXGIInfoQueue* dxgiInfoQueue;
		DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue));
		dxgiInfoQueue->GetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR);
		dxgiInfoQueue->GetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION);
		dxgiInfoQueue->Release();

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&s_Factory)), "Failed to create DXGI Factory")

		const D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
		IDXGIAdapter4* adapter;
		GetHardwareAdapter(s_Factory, &adapter, minFeatureLevel);

		DXGI_ADAPTER_DESC3 adapterDesc;
		adapter->GetDesc3(&adapterDesc);

		// Create device
		ThrowIfFailed(D3D12CreateDevice(adapter, minFeatureLevel, IID_PPV_ARGS(&s_Device)), "Failed to find a compatible device")
		s_Device->SetName(L"Main D3D12 Device");

		// Adapter no longer needed
		adapter->Release();

		{
			// Logging info
			ARC_CORE_INFO("DirectX Info:");
			_bstr_t wcDesc(adapterDesc.Description);
			const char* desc = wcDesc;
			ARC_CORE_INFO("  Vendor: {}", GetVendorName(adapterDesc.VendorId));
			ARC_CORE_INFO("  Renderer: {}", desc);
		}

#ifdef ARC_DEBUG
#ifdef ENABLE_DX12_DEBUG_MESSAGES
		HRESULT result = s_Device->QueryInterface(IID_PPV_ARGS(&s_InfoQueue1));
		
		if (SUCCEEDED(result))
		{
			s_InfoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			s_InfoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			s_InfoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			s_InfoQueue1->RegisterMessageCallback(DebugMessageCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &s_DebugCallbackCookie);
		}
		else
		{
			ARC_CORE_WARN("Could not enable enable DX12 debug messages on console window!");

			result = s_Device->QueryInterface(IID_PPV_ARGS(&s_InfoQueue));
			if (SUCCEEDED(result))
			{
				s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			}
			else
			{
				ARC_CORE_ERROR("Could not enable debugging support for the device!");
			}
		}
#else
		ARC_CORE_WARN("Support for DX12 debug messages on console window is disabled, define ENABLE_DX12_DEBUG_MESSAGES to enable the support, it requires Windows 11 SDK!");
		HRESULT result = s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
		if (SUCCEEDED(result))
		{
			s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			s_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		}
		else
		{
			ARC_CORE_ERROR("Could not enable debugging support for the DX12 device!");
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
			s_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&(frame.CommandAllocator)));
			s_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.CommandAllocator, nullptr, IID_PPV_ARGS(&(frame.CommandList)));

			std::string cmdAllocatorName = fmt::format("Command Allocator {}", tempInt);
			_bstr_t wCmdAllocatorName(cmdAllocatorName.c_str());
			frame.CommandAllocator->SetName(wCmdAllocatorName);
			std::string cmdListName = fmt::format("Command List {}", tempInt);
			_bstr_t wCmdListName(cmdListName.c_str());
			frame.CommandList->SetName(wCmdListName);
			frame.CommandList->Close();

			++tempInt;
		}
	}

	void Dx12Context::SwapBuffers()
	{
		ARC_PROFILE_SCOPE()

		auto& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];

		ID3D12CommandList* commandLists[] = { backFrame.CommandList };
		s_CommandQueue->ExecuteCommandLists(1, commandLists);

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
			s_Swapchain->ResizeBuffers(swapchainDesc.BufferCount, m_Width, m_Height, swapchainDesc.Format, swapchainDesc.Flags);
			CreateRTV();
			m_ShouldResize = false;
		}

		Dx12Frame::CurrentBackBuffer = s_Swapchain->GetCurrentBackBufferIndex();
	}

	void Dx12Context::SetSyncInterval(uint32_t value)
	{
		m_SyncInterval = value;
		m_PresentFlags |= m_SyncInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0;
	}

	ID3D12Device10* Dx12Context::GetDevice()
	{
		return s_Device;
	}

	ID3D12CommandAllocator* Dx12Context::GetCommandAllocator()
	{
		return s_Frames[Dx12Frame::CurrentBackBuffer].CommandAllocator;
	}

	ID3D12GraphicsCommandList7* Dx12Context::GetGraphicsCommandList()
	{
		return s_Frames[Dx12Frame::CurrentBackBuffer].CommandList;
	}

	DescriptorHeap* Dx12Context::GetSrvHeap()
	{
		return &s_SrvDescHeap;
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
		const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(m_Width), static_cast<float>(m_Height), 0.0f, 1.0f };
		const D3D12_RECT scissorRect = { 0, 0, static_cast<long>(m_Width), static_cast<long>(m_Height) };

		const auto& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];
		auto* commandList = backFrame.CommandList;

		backFrame.CommandAllocator->Reset();
		commandList->Reset(backFrame.CommandAllocator, nullptr);

		if (backFrame.DeferedReleasesFlag || backFrame.DeferedReleasesFlagHandles)
		{
			ProcessDeferredReleases(Dx12Frame::CurrentBackBuffer);
		}

		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backFrame.RtvBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		constexpr float clearColor[] = { 0.5f, 0.1f, 0.5f, 1.0f };
		commandList->ClearRenderTargetView(backFrame.RtvHandle.CPU, clearColor, 0, nullptr);
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
		backFrame.CommandList->ResourceBarrier(1, &barrier);
		backFrame.CommandList->Close();
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
		swapChainDesc.Width = m_Width;
		swapChainDesc.Height = m_Height;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
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
		m_Width = width;
		m_Height = height;
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
