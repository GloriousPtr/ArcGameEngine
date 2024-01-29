#include "arcpch.h"
#include "Dx12Context.h"

#include <dxgi1_6.h>
#include <d3d12.h>

#ifdef ARC_DEBUG
#include <dxgidebug.h>
#endif

#include <backends/imgui_impl_dx12.h>

#include "DxHelper.h"
#include "Dx12Resources.h"
#include "Dx12Allocator.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 608; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

namespace ArcEngine
{
	[[maybe_unused]] static const char* GetVendorName(UINT vendorId)
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

		eastl::vector<IUnknown*> DeferredReleases{};
		bool DeferedReleasesFlag = false;
		bool DeferedReleasesFlagHandles = false;

		inline static uint32_t CurrentBackBuffer = 0;
	};

	[[maybe_unused]] static void DebugMessageCallback(
		[[maybe_unused]] D3D12_MESSAGE_CATEGORY category,
		[[maybe_unused]] D3D12_MESSAGE_SEVERITY severity,
		[[maybe_unused]] D3D12_MESSAGE_ID id,
		[[maybe_unused]] LPCSTR pDescription,
		[[maybe_unused]] void* pContext)
	{
		ARC_PROFILE_SCOPE();

		switch (severity)
		{
			case D3D12_MESSAGE_SEVERITY_CORRUPTION:		ARC_CORE_CRITICAL(pDescription); return;
			case D3D12_MESSAGE_SEVERITY_ERROR:			ARC_CORE_ERROR(pDescription); return;
			case D3D12_MESSAGE_SEVERITY_WARNING:		ARC_CORE_WARN(pDescription); return;
			case D3D12_MESSAGE_SEVERITY_INFO:
			case D3D12_MESSAGE_SEVERITY_MESSAGE:
			default:
				break;
		}
	}

	inline static IDXGIFactory7* s_Factory;
	inline static IDXGIAdapter4* s_Adapter;
	inline static ID3D12Device11* s_Device;
	inline static IDXGISwapChain4* s_Swapchain;
	inline static ID3D12CommandQueue* s_CommandQueue;
	inline static eastl::vector<ID3D12GraphicsCommandList9*> s_CommandLists;
	inline static eastl::vector<ID3D12GraphicsCommandList9*> s_CommandListsInUse;
	inline static eastl::hash_map<ID3D12GraphicsCommandList9*, ID3D12CommandAllocator*> s_CommandListAllocatorMap;
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
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(hwnd, "Handle is null!");

		RECT clientRect;
		GetClientRect(m_Hwnd, &clientRect);
		s_Width = clientRect.right - clientRect.left;
		s_Height = clientRect.bottom - clientRect.top;
	}

	Dx12Context::~Dx12Context()
	{
		ARC_PROFILE_SCOPE();

		using namespace Microsoft::WRL;

		WaitForGpu();
		CloseHandle(s_Frames[Dx12Frame::CurrentBackBuffer].Fence.Event);

		for (auto& [cmdList, cmdAlloc] : s_CommandListAllocatorMap)
		{
			cmdList->Release();
			cmdAlloc->Release();
		}

		for (Dx12Frame& frame : s_Frames)
		{
			s_RtvDescHeap.Free(frame.RtvHandle);
			frame.RtvBuffer->Release();
			frame.Fence.Fence->Release();
		}

		for (uint32_t i = 0; i < FrameCount; ++i)
			ProcessDeferredReleases(i);

		s_RtvDescHeap.Release();
		s_DsvDescHeap.Release();
		s_SrvDescHeap.Release();
		s_UavDescHeap.Release();

		for (uint32_t i = 0; i < FrameCount; ++i)
			ProcessDeferredReleases(i);

		s_CommandQueue->Release();
		s_Swapchain->Release();
		Dx12Allocator::Shutdown();

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
#endif // ILLUMINO_DEBUG

		s_Device->Release();
		s_Adapter->Release();
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
		
		ComPtr<IDXGIDebug1> dxgiDebug = nullptr;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
	}

	void Dx12Context::Init()
	{
		ARC_PROFILE_SCOPE();

		using namespace Microsoft::WRL;

		UINT dxgiFactoryFlags = 0;

#ifdef ARC_DEBUG
		{
			ComPtr<ID3D12Debug3> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();

				ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
				DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue));
				dxgiInfoQueue->GetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING);
				dxgiInfoQueue->GetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR);
				dxgiInfoQueue->GetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION);

				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
			else
			{
				ARC_CORE_WARN("Failed to enable DirectX 12 debug layer. Make sure Graphics Tools (Apps & features/Optional features) is installed for it to work!");
			}
		}
#endif

		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&s_Factory)), "Failed to create DXGI Factory");

		const D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
		GetHardwareAdapter(s_Factory, &s_Adapter, &s_Device, minFeatureLevel);

		ARC_CORE_ASSERT(s_Adapter, "Failed to adapter");
		ARC_CORE_ASSERT(s_Device, "Failed to create device");
		s_Device->SetName(L"Main D3D12 Device");
		{
			// Logging info
			DXGI_ADAPTER_DESC3 adapterDesc;
			s_Adapter->GetDesc3(&adapterDesc);

			ARC_CORE_INFO("DirectX Info:");
			ARC_CORE_INFO("  Vendor: {}", GetVendorName(adapterDesc.VendorId));
			ARC_CORE_INFO(L"  Renderer: {}", adapterDesc.Description);
		}

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

		[[maybe_unused]] bool heapInitResult = true;
		heapInitResult &= s_RtvDescHeap.Init(4096, false);
		heapInitResult &= s_DsvDescHeap.Init(4096, false);
		heapInitResult &= s_SrvDescHeap.Init(8092, true);
		heapInitResult &= s_UavDescHeap.Init(2048, false);
		ARC_CORE_ASSERT(heapInitResult);

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
		s_CommandQueue->SetName(L"Main Command Queue");

		Dx12Allocator::Init(s_Adapter, s_Device);

		// Create Fences
		for (Dx12Frame& frame : s_Frames)
		{
			frame.Fence.Event = CreateEvent(nullptr, false, false, nullptr);
			frame.Fence.Value = 0;
			s_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&(frame.Fence.Fence)));
		}

		CreateSwapchain();

		// Create allocators and command lists
		for (uint32_t i = 0; i < 100; ++i)
		{
			ID3D12CommandAllocator* cmdAllocator;
			ID3D12GraphicsCommandList9* cmdList;
			s_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
			s_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));

			const std::string cmdAllocatorName = std::format("Graphics Command Allocator {}", i);
			NameResource(cmdAllocator, cmdAllocatorName.c_str());
			const std::string cmdListName = std::format("Graphics Command List {}", i);
			NameResource(cmdList, cmdListName.c_str());

			cmdList->Close();
			cmdAllocator->Reset();
			cmdList->Reset(cmdAllocator, nullptr);

			s_CommandLists.emplace_back(cmdList);
			s_CommandListAllocatorMap[cmdList] = cmdAllocator;
		}
	}

	void Dx12Context::SwapBuffers()
	{
		ARC_PROFILE_SCOPE();

		Dx12Frame& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];

		{
			ARC_PROFILE_SCOPE_NAME("ExecuteCommandLists");

			const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backFrame.RtvBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			for (ID3D12GraphicsCommandList9* cmdList : s_CommandListsInUse)
			{
				cmdList->ResourceBarrier(1, &barrier);
				cmdList->Close();
			}

			ID3D12CommandList** cmdListsInUse = reinterpret_cast<ID3D12CommandList**>(s_CommandListsInUse.data());
			s_CommandQueue->ExecuteCommandLists(static_cast<uint32_t>(s_CommandListsInUse.size()), cmdListsInUse);
		}

		WaitForGpu();

		{
			ARC_PROFILE_SCOPE_NAME("SwapchainPresent");
			s_Swapchain->Present(m_SyncInterval, m_PresentFlags);
			Dx12Frame::CurrentBackBuffer = s_Swapchain->GetCurrentBackBufferIndex();
		}

		if (m_ShouldResize)
		{
			ARC_PROFILE_SCOPE_NAME("ResizeViewport");

			WaitForGpu();
			for (Dx12Frame& frame : s_Frames)
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

		{
			ARC_PROFILE_SCOPE_NAME("CommandListsBookKeeping");
			ID3D12DescriptorHeap* descriptorHeap = GetSrvHeap()->Heap();
			for (ID3D12GraphicsCommandList9* cmdList : s_CommandListsInUse)
			{
				ID3D12CommandAllocator* cmdAllocator = s_CommandListAllocatorMap.at(cmdList);
				cmdAllocator->Reset();
				cmdList->Reset(cmdAllocator, nullptr);
				cmdList->SetDescriptorHeaps(1, &descriptorHeap);
			}
			s_CommandListsInUse.clear();
		}
	}

	void Dx12Context::SetSyncInterval(uint32_t value)
	{
		m_SyncInterval = value;
		m_PresentFlags = m_SyncInterval == 0 ?
							m_PresentFlags | DXGI_PRESENT_ALLOW_TEARING :
							m_PresentFlags & ~DXGI_PRESENT_ALLOW_TEARING;
	}

	ID3D12Device11* Dx12Context::GetDevice()
	{
		return s_Device;
	}

	ID3D12GraphicsCommandList9* Dx12Context::GetNewGraphicsCommandList()
	{
		ARC_CORE_ASSERT(s_CommandListsInUse.size() < s_CommandLists.size(), "Not enough command lists in pool");

		const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(s_Width), static_cast<float>(s_Height), 0.0f, 1.0f };
		const D3D12_RECT scissorRect = { 0, 0, static_cast<long>(s_Width), static_cast<long>(s_Height) };

		const Dx12Frame& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];
		auto* commandList = s_CommandLists[s_CommandListsInUse.size()];
		s_CommandListsInUse.emplace_back(commandList);

		if (backFrame.DeferedReleasesFlag || backFrame.DeferedReleasesFlagHandles)
		{
			ProcessDeferredReleases(Dx12Frame::CurrentBackBuffer);
		}

		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backFrame.RtvBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		commandList->OMSetRenderTargets(1, &(backFrame.RtvHandle.CPU), false, nullptr);

		ID3D12DescriptorHeap* descriptorHeaps[1] = { s_SrvDescHeap.Heap() };
		commandList->SetDescriptorHeaps(1, descriptorHeaps);
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		return commandList;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Dx12Context::GetRtv()
	{
		return s_Frames[Dx12Frame::CurrentBackBuffer].RtvHandle.CPU;
	}

	DescriptorHeap* Dx12Context::GetSrvHeap()
	{
		return &s_SrvDescHeap;
	}

	DescriptorHeap* Dx12Context::GetUavHeap()
	{
		return &s_UavDescHeap;
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

	void Dx12Context::Execute(ID3D12GraphicsCommandList9* commandList)
	{
		ARC_PROFILE_SCOPE();

		auto* it = eastl::find(s_CommandListsInUse.begin(), s_CommandListsInUse.end(), commandList);
		if (it != s_CommandListsInUse.end())
			s_CommandListsInUse.erase(it);

		auto* cmdAllocator = s_CommandListAllocatorMap.at(commandList);

		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(s_Frames[Dx12Frame::CurrentBackBuffer].RtvBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &barrier);
		commandList->Close();
		ID3D12CommandList* commandLists[] = { commandList };
		s_CommandQueue->ExecuteCommandLists(1, commandLists);
		WaitForGpu();
		cmdAllocator->Reset();
		commandList->Reset(cmdAllocator, nullptr);

		ID3D12DescriptorHeap* descriptorHeap = GetSrvHeap()->Heap();
		commandList->SetDescriptorHeaps(1, &descriptorHeap);
	}

	void Dx12Context::WaitForGpu()
	{
		ARC_PROFILE_SCOPE();

		Dx12Fence& fence = s_Frames[Dx12Frame::CurrentBackBuffer].Fence;
		UINT64& fenceValue = fence.Value;
		++fenceValue;

		s_CommandQueue->Signal(fence.Fence, fenceValue);
		fence.Fence->SetEventOnCompletion(fenceValue, fence.Event);
		WaitForSingleObject(fence.Event, INFINITE);
	}

	void Dx12Context::CreateRTV() const
	{
		ARC_PROFILE_SCOPE();

		// Create RTV
		int tempInt = 0;
		for (Dx12Frame& frame : s_Frames)
		{
			s_Swapchain->GetBuffer(tempInt, IID_PPV_ARGS(&(frame.RtvBuffer)));
			frame.RtvHandle = s_RtvDescHeap.Allocate();
			s_Device->CreateRenderTargetView(frame.RtvBuffer, nullptr, frame.RtvHandle.CPU);

			const std::string rtvFrameName = std::format("RTV Frame {}", tempInt);
			NameResource(frame.RtvBuffer, rtvFrameName.c_str());

			++tempInt;
		}
	}

	void Dx12Context::CreateSwapchain() const
	{
		ARC_PROFILE_SCOPE();

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
		ARC_PROFILE_SCOPE();

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
			eastl::vector<IUnknown*>& resources = backFrame.DeferredReleases;
			for (IUnknown* resource : resources)
			{
				resource->Release();
			}
			resources.clear();
		}
	}

	void Dx12Context::DeferredRelease(IUnknown* resource)
	{
		Dx12Frame& backFrame = s_Frames[Dx12Frame::CurrentBackBuffer];
		std::lock_guard lock(s_DeferredReleasesMutex);
		backFrame.DeferredReleases.push_back(resource);
		backFrame.DeferedReleasesFlag = true;
	}
}
