#include "arcpch.h"
#include "Arc/ImGui/ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Arc/Core/Application.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <ImGuizmo.h>

namespace ArcEngine
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	void ImGuiLayer::OnAttach()
	{
		ARC_PROFILE_SCOPE();
		
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

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
		
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach()
	{
		ARC_PROFILE_SCOPE();
		
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
		ARC_PROFILE_SCOPE();
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
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
