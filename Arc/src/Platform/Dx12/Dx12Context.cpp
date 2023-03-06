#include "arcpch.h"
#include "Dx12Context.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comutil.h>
#include "d3dx12.h"

#ifdef ARC_DEBUG
#include <dxgidebug.h>
#endif

#include "DxHelper.h"

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
		ID3D12Fence* Fence;
		uint64_t Value;
		HANDLE Event;
	};

	struct Dx12Frame
	{
		Dx12Fence Fence;
		ID3D12Resource* RtvBuffer;
		ID3D12CommandAllocator* CommandAllocator;
		ID3D12GraphicsCommandList7* CommandList;

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

	inline static ID3D12Device10* s_Device;
	inline static ID3D12CommandQueue* s_CommandQueue;
	inline static IDXGISwapChain4* s_Swapchain;

	inline static ID3D12DescriptorHeap* s_RtvHeap;
	inline static Dx12Frame s_Frames[Dx12Context::FrameCount];

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

#ifdef ARC_DEBUG
#ifdef ENABLE_DX12_DEBUG_MESSAGES
		ID3D12InfoQueue1* infoQueue1;
		HRESULT hr = s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue1));

		if (SUCCEEDED(hr))
		{
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			infoQueue1->UnregisterMessageCallback(s_DebugCallbackCookie);
			infoQueue1->Release();
		}
		else
		{
			ID3D12InfoQueue* infoQueue;
			hr = s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
			if (SUCCEEDED(hr))
			{
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
				infoQueue->Release();
			}
		}
#else
		ID3D12InfoQueue* infoQueue;
		HRESULT hr = s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
		if (SUCCEEDED(hr))
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			infoQueue->Release();
		}
#endif // ENABLE_DX12_DEBUG_MESSAGES
#endif // ILLUMINO_DEBUG

		for (const auto& frame : s_Frames)
		{
			frame.RtvBuffer->Release();
			frame.Fence.Fence->Release();
			frame.CommandList->Release();
			frame.CommandAllocator->Release();
		}

		s_RtvHeap->Release();
		s_Swapchain->Release();
		s_CommandQueue->Release();
		s_Device->Release();
	}

	void Dx12Context::Init()
	{
		ARC_PROFILE_SCOPE()

		UINT dxgiFactoryFlags = 0;

#ifdef ARC_DEBUG
		ID3D12Debug* debug;
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

		IDXGIFactory7* factory;
		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)), "Failed to create DXGI Factory")

		const D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
		IDXGIAdapter4* adapter;
		GetHardwareAdapter(factory, &adapter, minFeatureLevel);

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
		ID3D12InfoQueue1* infoQueue1;
		HRESULT hr = s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue1));
		
		if (SUCCEEDED(hr))
		{
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			infoQueue1->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue1->RegisterMessageCallback(DebugMessageCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr, &s_DebugCallbackCookie);
			infoQueue1->Release();
		}
		else
		{
			ARC_CORE_WARN("Could not enable enable DX12 debug messages on console window!");

			ID3D12InfoQueue* infoQueue;
			hr = s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
			if (SUCCEEDED(hr))
			{
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
				infoQueue->Release();
			}
			else
			{
				ARC_CORE_ERROR("Could not enable debugging support for the device!");
			}
		}
#else
		ARC_CORE_WARN("Support for DX12 debug messages on console window is disabled, define ENABLE_DX12_DEBUG_MESSAGES to enable the support, it requires Windows 11 SDK!");
		ID3D12InfoQueue* infoQueue;
		HRESULT hr = s_Device->QueryInterface(IID_PPV_ARGS(&infoQueue));
		if (SUCCEEDED(hr))
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue->Release();
		}
		else
		{
			ARC_CORE_ERROR("Could not enable debugging support for the DX12 device!");
		}
#endif // ENABLE_DX12_DEBUG_MESSAGES
#endif // ARC_CORE_DEBUG

		// Create command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 0;
		ThrowIfFailed(s_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&s_CommandQueue)), "Failed to create command queue");
		s_CommandQueue->SetName(L"Main D3D12 Command Queue");

		// Create Swapchain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = m_Width;
		swapChainDesc.Height = m_Height;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		IDXGISwapChain1* newSwapchain;
		if (SUCCEEDED(factory->CreateSwapChainForHwnd(s_CommandQueue, m_Hwnd, &swapChainDesc, nullptr, nullptr, &newSwapchain)))
			s_Swapchain = static_cast<IDXGISwapChain4*>(newSwapchain);

		// Create Fences
		for (auto& frame : s_Frames)
		{
			frame.Fence.Event = CreateEvent(nullptr, false, false, nullptr);
			frame.Fence.Value = 0;
			s_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&(frame.Fence.Fence)));
		}

		// Create RTVHeap
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		s_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&s_RtvHeap));
		s_RtvHeap->SetName(L"Main RTV Heap");

		// Create RTV
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(s_RtvHeap->GetCPUDescriptorHandleForHeapStart());
		int tempInt = 0;
		for (auto& frame : s_Frames)
		{
			s_Swapchain->GetBuffer(tempInt, IID_PPV_ARGS(&(frame.RtvBuffer)));
			s_Device->CreateRenderTargetView(frame.RtvBuffer, nullptr, rtvHandle);
			rtvHandle.Offset(1, s_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

			std::string rtvFrameName = fmt::format("RTV Frame {}", tempInt);
			_bstr_t wRtvFrameName(rtvFrameName.c_str());
			frame.RtvBuffer->SetName(wRtvFrameName);

			++tempInt;
		}

		// Factory no longer needed
		factory->Release();

		// Create allocators and command lists
		tempInt = 0;
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

		Dx12Frame::CurrentBackBuffer = s_Swapchain->GetCurrentBackBufferIndex();
	}

	void Dx12Context::SwapBuffers()
	{
		ARC_PROFILE_SCOPE()

		auto& frame = s_Frames[Dx12Frame::CurrentBackBuffer];

		{
			frame.CommandAllocator->Reset();

			frame.CommandList->Reset(frame.CommandAllocator, nullptr);

			frame.CommandList->Close();
		}

		ID3D12CommandList* commandLists[] = { frame.CommandList };
		s_CommandQueue->ExecuteCommandLists(1, commandLists);

		s_Swapchain->Present(m_SyncInterval, m_PresentFlags);

		const auto fenceValue = frame.Fence.Value;
		s_CommandQueue->Signal(frame.Fence.Fence, fenceValue);
		++frame.Fence.Value;

		WaitForFence(frame.Fence.Fence, fenceValue, frame.Fence.Event);

		Dx12Frame::CurrentBackBuffer = s_Swapchain->GetCurrentBackBufferIndex();
	}

	void Dx12Context::SetSyncInterval(uint32_t value)
	{
		m_SyncInterval = value;
		m_PresentFlags |= m_SyncInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0;
	}
}
