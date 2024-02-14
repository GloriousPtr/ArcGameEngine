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

	struct Dx12Frame
	{
		ID3D12Resource* RtvBuffer = nullptr;
		DescriptorHandle RtvHandle{};

		eastl::vector<IUnknown*> DeferredReleases{};
		bool DeferedReleasesFlag = false;
		bool DeferedReleasesFlagHandles = false;
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
	inline static eastl::stack<D3D12GraphicsCommandList*> s_CommandLists[Dx12Context::FrameCount];
	inline static eastl::vector<D3D12GraphicsCommandList*> s_CommandListsToSubmit[Dx12Context::FrameCount];
	inline static eastl::hash_map<D3D12GraphicsCommandList*, ID3D12CommandAllocator*> s_CommandListAllocatorMap[Dx12Context::FrameCount];
	inline static std::mutex s_CommandListMtx;
	inline static Dx12Frame s_Frames[Dx12Context::FrameCount];

	inline static ID3D12Fence* s_Fence = nullptr;
	inline static HANDLE s_FenceEvent;
	inline static uint64_t s_FenceValues[Dx12Context::FrameCount]{};
	inline static uint32_t s_FrameIndex = 0;

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
		CloseHandle(s_FenceEvent);

		for (auto& cmdListAllocMap : s_CommandListAllocatorMap)
		{
			for (auto& [cmdList, cmdAlloc] : cmdListAllocMap)
			{
				cmdList->Release();
				cmdAlloc->Release();
			}
		}

		for (Dx12Frame& frame : s_Frames)
		{
			s_RtvDescHeap.Free(frame.RtvHandle);
			frame.RtvBuffer->Release();
		}

		s_Fence->Release();

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

		CreateSwapchain();

		// Create Fence
		s_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
		s_Device->CreateFence(s_FenceValues[s_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&s_Fence));
		s_FenceValues[s_FrameIndex]++;

		// Create allocators and command lists
		for (uint32_t frameIdx = 0; frameIdx < Dx12Context::FrameCount; ++frameIdx)
		{
			for (uint32_t i = 0; i < 1000; ++i)
			{
				ID3D12CommandAllocator* cmdAllocator;
				D3D12GraphicsCommandList* cmdList;
				s_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
				s_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));

				const std::string cmdAllocatorName = std::format("Graphics Command Allocator {}", i);
				NameResource(cmdAllocator, cmdAllocatorName.c_str());
				const std::string cmdListName = std::format("Graphics Command List {}", i);
				NameResource(cmdList, cmdListName.c_str());

				cmdList->Close();

				s_CommandLists[frameIdx].push(cmdList);
				s_CommandListAllocatorMap[frameIdx][cmdList] = cmdAllocator;
			}
		}
	}

	void Dx12Context::SwapBuffers()
	{
		ARC_PROFILE_SCOPE();

		Flush_Internal(true);

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
	}

	void Dx12Context::SetSyncInterval(uint32_t value)
	{
		m_SyncInterval = value;
		m_PresentFlags = m_SyncInterval == 0 ?
							m_PresentFlags | DXGI_PRESENT_ALLOW_TEARING :
							m_PresentFlags & ~DXGI_PRESENT_ALLOW_TEARING;
	}

	void Dx12Context::GetStats(Stats& stats)
	{
		D3D12MA::Budget budget;
		Dx12Allocator::GetStats(budget);
		stats.BlockCount = budget.Stats.BlockCount;
		stats.AllocationCount = budget.Stats.AllocationCount;
		stats.BlockBytes = budget.Stats.BlockBytes;
		stats.AllocationBytes = budget.Stats.AllocationBytes;
		stats.UsageBytes = budget.UsageBytes;
		stats.BudgetBytes = budget.BudgetBytes;
	}

	ID3D12Device11* Dx12Context::GetDevice()
	{
		return s_Device;
	}

	GraphicsCommandList Dx12Context::BeginRecordingCommandList()
	{
		const Dx12Frame& backFrame = s_Frames[s_FrameIndex];
		const uint32_t frameIndex = s_FrameIndex;

		s_CommandListMtx.lock();
		ARC_CORE_ASSERT(s_CommandLists[frameIndex].size() > 0, "Not enough command lists in pool");
		D3D12GraphicsCommandList* commandList = s_CommandLists[frameIndex].top();
		s_CommandLists[frameIndex].pop();
		ID3D12CommandAllocator* cmdAllocator = s_CommandListAllocatorMap[frameIndex].at(commandList);
		s_CommandListMtx.unlock();

		const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(s_Width), static_cast<float>(s_Height), 0.0f, 1.0f };
		const D3D12_RECT scissorRect = { 0, 0, static_cast<long>(s_Width), static_cast<long>(s_Height) };

		cmdAllocator->Reset();
		commandList->Reset(cmdAllocator, nullptr);

		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backFrame.RtvBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		commandList->OMSetRenderTargets(1, &(backFrame.RtvHandle.CPU), false, nullptr);

		ID3D12DescriptorHeap* descriptorHeaps[1] = { s_SrvDescHeap.Heap() };
		commandList->SetDescriptorHeaps(1, descriptorHeaps);
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		return commandList;
	}

	void Dx12Context::EndRecordingCommandList(GraphicsCommandList commandList, bool execute)
	{
		if (execute)
		{
			ExecuteCommandList(commandList);
		}
		else
		{
			D3D12GraphicsCommandList* cmdList = reinterpret_cast<D3D12GraphicsCommandList*>(commandList);
			const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(s_Frames[s_FrameIndex].RtvBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			cmdList->ResourceBarrier(1, &barrier);
			cmdList->Close();
			s_CommandListMtx.lock();
			s_CommandListsToSubmit[s_FrameIndex].emplace_back(cmdList);
			s_CommandListMtx.unlock();
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Dx12Context::GetRtv()
	{
		return s_Frames[s_FrameIndex].RtvHandle.CPU;
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
		return s_FrameIndex;
	}

	void Dx12Context::SetDeferredReleaseFlag()
	{
		s_Frames[s_FrameIndex].DeferedReleasesFlagHandles = true;
	}

	void Dx12Context::ExecuteCommandList(GraphicsCommandList commandList)
	{
		ARC_PROFILE_SCOPE();
		
		D3D12GraphicsCommandList* cmdList = reinterpret_cast<D3D12GraphicsCommandList*>(commandList);
		
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(s_Frames[s_FrameIndex].RtvBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		cmdList->ResourceBarrier(1, &barrier);
		cmdList->Close();
		ID3D12CommandList* commandLists[] = { cmdList };
		s_CommandQueue->ExecuteCommandLists(1, commandLists);
		WaitForGpu();

		s_CommandListMtx.lock();
		s_CommandLists[s_FrameIndex].push(cmdList);
		s_CommandListMtx.unlock();
	}

	void Dx12Context::WaitForGpu()
	{
		ARC_PROFILE_CATEGORY("WaitForGpu", ArcEngine::Profile::Category::Wait);

		uint64_t& fenceValue = s_FenceValues[s_FrameIndex];
		s_CommandQueue->Signal(s_Fence, fenceValue);
		s_Fence->SetEventOnCompletion(fenceValue, s_FenceEvent);
		WaitForSingleObjectEx(s_FenceEvent, INFINITE, false);

		s_FenceValues[s_FrameIndex]++;
	}

	void Dx12Context::Flush()
	{
		Flush_Internal(false);
	}

	void Dx12Context::Flush_Internal(bool swapBuffers)
	{
		const uint32_t frameIndex = s_FrameIndex;

		if (s_CommandListsToSubmit[frameIndex].empty())
			return;

		{
			ARC_PROFILE_SCOPE_NAME("ExecuteCommandLists");
			ID3D12CommandList** cmdListsInUse = reinterpret_cast<ID3D12CommandList**>(s_CommandListsToSubmit[frameIndex].data());
			s_CommandQueue->ExecuteCommandLists(static_cast<uint32_t>(s_CommandListsToSubmit[frameIndex].size()), cmdListsInUse);
		}

		if (swapBuffers)
		{
			ARC_PROFILE_CATEGORY("Present", ArcEngine::Profile::Category::Wait);
			s_Swapchain->Present(m_SyncInterval, m_PresentFlags);
		}

		// Move to next frame
		{
			ARC_PROFILE_SCOPE_NAME("MoveToNextFrame");

			const UINT64 currentFenceValue = s_FenceValues[s_FrameIndex];
			s_CommandQueue->Signal(s_Fence, currentFenceValue);
			s_FrameIndex = s_Swapchain->GetCurrentBackBufferIndex();
			if (s_Fence->GetCompletedValue() < s_FenceValues[s_FrameIndex])
			{
				s_Fence->SetEventOnCompletion(s_FenceValues[s_FrameIndex], s_FenceEvent);
				WaitForSingleObjectEx(s_FenceEvent, INFINITE, false);
			}
			s_FenceValues[s_FrameIndex] = currentFenceValue + 1;
		}

		{
			ARC_PROFILE_SCOPE_NAME("CommandListsBookKeeping");
			s_CommandListMtx.lock();
			for (D3D12GraphicsCommandList* cmdList : s_CommandListsToSubmit[frameIndex])
			{
				s_CommandLists[frameIndex].push(cmdList);
			}
			s_CommandListsToSubmit[frameIndex].clear();
			s_CommandListMtx.unlock();
		}

		const Dx12Frame& backFrame = s_Frames[s_FrameIndex];
		if (backFrame.DeferedReleasesFlag || backFrame.DeferedReleasesFlagHandles)
		{
			WaitForGpu();
			ProcessDeferredReleases(s_FrameIndex);
		}
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
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		IDXGISwapChain1* newSwapchain;
		s_Factory->CreateSwapChainForHwnd(s_CommandQueue, m_Hwnd, &swapChainDesc, nullptr, nullptr, &newSwapchain);
		s_Swapchain = reinterpret_cast<IDXGISwapChain4*>(newSwapchain);
		
		CreateRTV();

		s_FrameIndex = s_Swapchain->GetCurrentBackBufferIndex();
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
		Dx12Frame& backFrame = s_Frames[s_FrameIndex];
		std::lock_guard lock(s_DeferredReleasesMutex);
		backFrame.DeferredReleases.push_back(resource);
		backFrame.DeferedReleasesFlag = true;
	}
}
