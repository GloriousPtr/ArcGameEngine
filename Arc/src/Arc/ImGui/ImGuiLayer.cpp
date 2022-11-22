#include "arcpch.h"
#include "ImGuiLayer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <ImGuizmo.h>

#include "Arc/Core/Application.h"

namespace ArcEngine
{
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

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

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
		
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent([[maybe_unused]] Event& e)
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
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End() const
	{
		ARC_PROFILE_SCOPE();
		
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		{
			ARC_PROFILE_SCOPE("ImGui::End::Render");
			// Rendering
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
}
