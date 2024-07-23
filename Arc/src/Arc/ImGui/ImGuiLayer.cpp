#include "arcpch.h"
#include "ImGuiLayer.h"

#ifdef ARC_PLATFORM_LINUX
#endif
#ifdef ARC_PLATFORM_WINDOWS
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx12.h>
#include <d3d12.h>
#include "Platform/Dx12/Dx12Context.h"
#include "Platform/Dx12/Dx12Resources.h"
#endif

#include <imgui.h>
#include <ImGuizmo.h>

#include "Arc/Core/Application.h"

namespace ArcEngine
{
#ifdef ARC_PLATFORM_WINDOWS
	static inline DescriptorHandle s_DescriptorHandle{};
#endif

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	void ImGuiLayer::OnAttach()
	{
		ARC_PROFILE_SCOPE();
		
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.ConfigDragClickToInputText = true;
		io.ConfigDockingTransparentPayload = true;

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

#ifdef ARC_PLATFORM_LINUX
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		// Setup Platform/Renderer backends
#endif
#ifdef ARC_PLATFORM_WINDOWS
		auto hwnd = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto* device = Dx12Context::GetDevice();
		auto* srvHeap = Dx12Context::GetSrvHeap();
		s_DescriptorHandle = srvHeap->Allocate();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOther(hwnd, true);
		ImGui_ImplDX12_Init(device,	Dx12Context::FrameCount, static_cast<DXGI_FORMAT>(Dx12Context::GetSwapChainFormat()),
			srvHeap->Heap(), s_DescriptorHandle.CPU, s_DescriptorHandle.GPU);
#endif

		{
			ImGuizmo::Style* imguizmoStyle  = &ImGuizmo::GetStyle();
			ImVec4* colors = imguizmoStyle->Colors;

			imguizmoStyle->TranslationLineThickness = 3.0f;
			imguizmoStyle->TranslationLineArrowSize = 10.0f;
			imguizmoStyle->RotationLineThickness = 3.0f;
			imguizmoStyle->RotationOuterLineThickness = 5.0f;
			imguizmoStyle->ScaleLineThickness = 3.0f;
			imguizmoStyle->ScaleLineCircleSize = 8.0f;
			imguizmoStyle->HatchedAxisLineThickness = 0.0f;
			imguizmoStyle->CenterCircleSize = 6.0f;
			
			colors[ImGuizmo::DIRECTION_X] = ImVec4(0.858f, 0.243f, 0.113f, 0.929f);
			colors[ImGuizmo::DIRECTION_Y] = ImVec4(0.375f, 0.825f, 0.372f, 0.929f);
			colors[ImGuizmo::DIRECTION_Z] = ImVec4(0.227f, 0.478f, 0.972f, 0.929f);
			colors[ImGuizmo::PLANE_X] = colors[ImGuizmo::DIRECTION_X];
			colors[ImGuizmo::PLANE_Y] = colors[ImGuizmo::DIRECTION_Y];
			colors[ImGuizmo::PLANE_Z] = colors[ImGuizmo::DIRECTION_Z];
		}
	}

	void ImGuiLayer::OnDetach()
	{
		ARC_PROFILE_SCOPE();

		Dx12Context::WaitForGpu();

#ifdef ARC_PLATFORM_LINUX
#endif
#ifdef ARC_PLATFORM_WINDOWS
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplGlfw_Shutdown();
#endif

		ImGui::DestroyContext();

		Dx12Context::GetSrvHeap()->Free(s_DescriptorHandle);
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if(m_BlockEvents)
		{
			const ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin() const
	{
		ARC_PROFILE_SCOPE();

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End() const
	{
		ARC_PROFILE_SCOPE();

		ImGuiIO& io = ImGui::GetIO();
		const Window& window = Application::Get().GetWindow();
		io.DisplaySize = ImVec2(static_cast<float>(window.GetWidth()), static_cast<float>(window.GetHeight()));

#ifdef ARC_PLATFORM_WINDOWS
		GraphicsCommandList commandList = Dx12Context::BeginRecordingCommandList();
#endif

		{
			ARC_PROFILE_SCOPE_NAME("ImGui::End::Render");
			// Rendering
			ImGui::Render();
#ifdef ARC_PLATFORM_LINUX
#endif
#ifdef ARC_PLATFORM_WINDOWS
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), reinterpret_cast<ID3D12GraphicsCommandList*>(commandList));
#endif
		}

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
#ifdef ARC_PLATFORM_LINUX
#endif
#ifdef ARC_PLATFORM_WINDOWS
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, commandList);
			Dx12Context::EndRecordingCommandList(commandList);
#endif
		}
	}
}
