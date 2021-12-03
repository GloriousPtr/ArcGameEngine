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
		ImGuiIO& io = ImGui::GetIO();
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
		io.Fonts->Build();
		
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		const ImVec4 white_a_1 = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
		const ImVec4 white_a_156 = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
		const ImVec4 white_a_0 = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);

		const ImVec4 lightBlue = ImVec4(0.549f, 0.976f, 1.000f, 1.000f);
		
		const ImVec4 blue = ImVec4(0.317f, 0.882f, 0.910f, 1.000f); //ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
		const ImVec4 blue_a = ImVec4(0.317f, 0.882f, 0.910f, 0.781f);
		
		const ImVec4 gray_469 = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
		const ImVec4 gray_391 = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
		const ImVec4 gray_195 = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
		const ImVec4 gray_180 = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
		const ImVec4 gray_098 = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);

		
		colors[ImGuiCol_Text] = white_a_1;
		colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(lightBlue.x, lightBlue.y, lightBlue.z, 0.156f); //white_a_156;
		
		colors[ImGuiCol_WindowBg] = gray_180;
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
		
		colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
		
		colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
		
		colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
		
		colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
		
		colors[ImGuiCol_MenuBarBg] = gray_195;
		
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
		colors[ImGuiCol_ScrollbarGrabActive] = blue;
		
		colors[ImGuiCol_CheckMark] = white_a_1;
		
		colors[ImGuiCol_SliderGrab] = gray_391;
		colors[ImGuiCol_SliderGrabActive] = blue;
		
		colors[ImGuiCol_Button] = white_a_0;
		colors[ImGuiCol_ButtonHovered] = ImVec4(lightBlue.x, lightBlue.y, lightBlue.z, 0.156f);
		colors[ImGuiCol_ButtonActive] = ImVec4(lightBlue.x, lightBlue.y, lightBlue.z, 0.391f);
		
		colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
		colors[ImGuiCol_HeaderHovered] = gray_469;
		colors[ImGuiCol_HeaderActive] = gray_469;
		
		colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
		colors[ImGuiCol_SeparatorHovered] = gray_391;
		colors[ImGuiCol_SeparatorActive] = blue;
		
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
		colors[ImGuiCol_ResizeGripActive] = blue;
		
		colors[ImGuiCol_Tab] = gray_098;
		colors[ImGuiCol_TabHovered] = ImVec4(lightBlue.x, lightBlue.y, lightBlue.z, 0.391f);// ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
		colors[ImGuiCol_TabActive] = gray_195;
		colors[ImGuiCol_TabUnfocused] = gray_098;
		colors[ImGuiCol_TabUnfocusedActive] = gray_195;
		
		colors[ImGuiCol_DockingPreview] = blue_a;
		colors[ImGuiCol_DockingEmptyBg] = gray_180;
		
		colors[ImGuiCol_PlotLines] = blue; //gray_469;
		colors[ImGuiCol_PlotLinesHovered] = lightBlue;
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
		colors[ImGuiCol_PlotHistogramHovered] = blue;
		
		colors[ImGuiCol_DragDropTarget] = blue;
		
		colors[ImGuiCol_NavHighlight] = blue;
		colors[ImGuiCol_NavWindowingHighlight] = blue;
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
		

		
		style->ChildRounding = 4.0f;
		style->FrameBorderSize = 1.0f;
		style->FrameRounding = 2.0f;
		style->GrabMinSize = 7.0f;
		style->PopupRounding = 2.0f;
		style->ScrollbarRounding = 12.0f;
		style->ScrollbarSize = 13.0f;
		style->TabBorderSize = 1.0f;
		style->TabRounding = 0.0f;
		style->WindowRounding = 4.0f;
	}
}
