#include "arcpch.h"
#include "Arc/ImGui/ImGuiLayer.h"

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include "Arc/Core/Application.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <ImGuizmo.h>

namespace ArcEngine
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	void ImGuiLayer::OnAttach()
	{
		ARC_PROFILE_FUNCTION();
		
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		io.ConfigWindowsMoveFromTitleBarOnly = true;

		/*
		io.AddInputCharacter(GLFW_KEY_SPACE);
	    io.AddInputCharacter(GLFW_KEY_APOSTROPHE);
	    io.AddInputCharacter(GLFW_KEY_COMMA);
	    io.AddInputCharacter(GLFW_KEY_MINUS);
	    io.AddInputCharacter(GLFW_KEY_PERIOD);
	    io.AddInputCharacter(GLFW_KEY_SLASH);
	    io.AddInputCharacter(GLFW_KEY_0);
	    io.AddInputCharacter(GLFW_KEY_1);
	    io.AddInputCharacter(GLFW_KEY_2);
	    io.AddInputCharacter(GLFW_KEY_3);
	    io.AddInputCharacter(GLFW_KEY_4);
	    io.AddInputCharacter(GLFW_KEY_5);
	    io.AddInputCharacter(GLFW_KEY_6);
	    io.AddInputCharacter(GLFW_KEY_7);
	    io.AddInputCharacter(GLFW_KEY_8);
	    io.AddInputCharacter(GLFW_KEY_9);
	    io.AddInputCharacter(GLFW_KEY_SEMICOLON);
	    io.AddInputCharacter(GLFW_KEY_EQUAL);
	    io.AddInputCharacter(GLFW_KEY_A);
	    io.AddInputCharacter(GLFW_KEY_B);
	    io.AddInputCharacter(GLFW_KEY_C);
	    io.AddInputCharacter(GLFW_KEY_D);
	    io.AddInputCharacter(GLFW_KEY_E);
	    io.AddInputCharacter(GLFW_KEY_F);
	    io.AddInputCharacter(GLFW_KEY_G);
	    io.AddInputCharacter(GLFW_KEY_H);
	    io.AddInputCharacter(GLFW_KEY_I);
	    io.AddInputCharacter(GLFW_KEY_J);
	    io.AddInputCharacter(GLFW_KEY_K);
	    io.AddInputCharacter(GLFW_KEY_L);
	    io.AddInputCharacter(GLFW_KEY_M);
	    io.AddInputCharacter(GLFW_KEY_N);
	    io.AddInputCharacter(GLFW_KEY_O);
	    io.AddInputCharacter(GLFW_KEY_P);
	    io.AddInputCharacter(GLFW_KEY_Q);
	    io.AddInputCharacter(GLFW_KEY_R);
	    io.AddInputCharacter(GLFW_KEY_S);
	    io.AddInputCharacter(GLFW_KEY_T);
	    io.AddInputCharacter(GLFW_KEY_U);
	    io.AddInputCharacter(GLFW_KEY_V);
	    io.AddInputCharacter(GLFW_KEY_W);
	    io.AddInputCharacter(GLFW_KEY_X);
	    io.AddInputCharacter(GLFW_KEY_Y);
	    io.AddInputCharacter(GLFW_KEY_Z);
	    io.AddInputCharacter(GLFW_KEY_LEFT_BRACKET);
	    io.AddInputCharacter(GLFW_KEY_BACKSLASH);
	    io.AddInputCharacter(GLFW_KEY_RIGHT_BRACKET);
	    io.AddInputCharacter(GLFW_KEY_GRAVE_ACCENT);
	    io.AddInputCharacter(GLFW_KEY_WORLD_1);
	    io.AddInputCharacter(GLFW_KEY_WORLD_2);
		*/
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
		
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();
		
		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach()
	{
		ARC_PROFILE_FUNCTION();
		
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if(m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin()
	{
		ARC_PROFILE_FUNCTION();
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		ARC_PROFILE_FUNCTION();
		
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Checkbox
		colors[ImGuiCol_CheckMark] = ImVec4{ 0.8f, 0.8f, 0.8f, 1.0f };
	}
}
