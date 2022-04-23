#include "EditorLayer.h"

#include <imgui/imgui.h>

#include "Arc/ImGui/Modules/ImGuiConsole.h"
#include "Arc/Scene/SceneSerializer.h"
#include "Arc/Utils/PlatformUtils.h"
#include "Arc/Math/Math.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/RendererSettingsPanel.h"
#include "Panels/StatsPanel.h"

namespace ArcEngine
{
	EditorLayer::EditorLayer()
		: Layer("Sandbox2D")
	{
	}

	void EditorLayer::OnAttach()
	{
		ARC_PROFILE_FUNCTION();

		m_Application = &Application::Get();
		m_ActiveScene = CreateRef<Scene>();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_Viewports.push_back(CreateScope<SceneViewport>());
		m_Viewports[0]->SetSceneHierarchyPanel(m_SceneHierarchyPanel);
		
		m_Properties.push_back(CreateScope<PropertiesPanel>());

		m_Panels.push_back(CreateScope<RendererSettingsPanel>());
		m_Panels.push_back(CreateScope<StatsPanel>());
	}

	void EditorLayer::OnDetach()
	{
		ARC_PROFILE_FUNCTION();
		
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		OPTICK_EVENT("EditorUpdate");

		ARC_PROFILE_FUNCTION();

		Renderer2D::ResetStats();
		Renderer3D::ResetStats();

		for (size_t i = 0; i < m_Viewports.size(); i++)
		{
			if (m_Viewports[i]->IsShowing())
				m_Viewports[i]->OnUpdate(m_ActiveScene, ts);
		}

		for (size_t i = 0; i < m_Panels.size(); i++)
		{
			if (m_Panels[i]->IsShowing())
				m_Panels[i]->OnUpdate(ts);
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		OPTICK_EVENT("EditorRender");

		ARC_PROFILE_FUNCTION();

		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;
		
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					NewScene();
				}

				if (ImGui::MenuItem("Open..", "Ctrl+O"))
				{
					OpenScene();
				}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}

				if (ImGui::MenuItem("Exit"))
				{
					m_Application->Close();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::BeginMenu("Add"))
				{
					if (ImGui::MenuItem("Viewport"))
					{
						size_t index = m_Viewports.size();
						m_Viewports.push_back(CreateScope<SceneViewport>());
						m_Viewports[index]->SetSceneHierarchyPanel(m_SceneHierarchyPanel);
					}

					if (ImGui::MenuItem("Properties"))
					{
						m_Properties.push_back(CreateScope<PropertiesPanel>());
					}

					ImGui::EndMenu();
				}
				ImGui::MenuItem("Hierarchy", nullptr, &m_ShowSceneHierarchyPanel);
				ImGui::MenuItem("Console", nullptr, &m_ShowConsole);

				for (size_t i = 0; i < m_Panels.size(); i++)
				{
					BasePanel* panel = m_Panels[i].get();
					bool showing = panel->IsShowing();
					if (ImGui::MenuItem(panel->GetName(), nullptr, &showing))
						panel->SetShowing(showing);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Shaders"))
			{
				if (ImGui::MenuItem("Reload Shaders"))
				{
					Renderer3D::GetShaderLibrary().ReloadAll();
					Renderer3D::Init();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Remove unused scene viewports
		for (auto it = m_Viewports.begin(); it != m_Viewports.end(); it++)
		{
			if (!it->get()->IsShowing())
			{
				m_Viewports.erase(it);
				break;
			}
		}

		// Remove unused properties panels
		for (auto it = m_Properties.begin(); it != m_Properties.end(); it++)
		{
			if (!it->get()->IsShowing())
			{
				m_Properties.erase(it);
				break;
			}
		}

		if (m_ShowSceneHierarchyPanel)
			m_SceneHierarchyPanel.OnImGuiRender();
		Entity selectedContext = m_SceneHierarchyPanel.GetSelectedEntity();

		// Show viewports
		for (size_t i = 0; i < m_Viewports.size(); i++)
		{
			if (m_Viewports[i]->IsShowing())
				m_Viewports[i]->OnImGuiRender();
		}

		// Show properties panel
		for (size_t i = 0; i < m_Properties.size(); i++)
		{
			if (m_Properties[i]->IsShowing())
			{
				m_Properties[i]->SetContext(selectedContext);
				m_Properties[i]->OnImGuiRender();
			}
		}

		// Show panels
		for (size_t i = 0; i < m_Panels.size(); i++)
		{
			if (m_Panels[i]->IsShowing())
				m_Panels[i]->OnImGuiRender();
		}

		if (m_ShowConsole)
			ImGuiConsole::OnImGuiRender(&m_ShowConsole);

		m_Application->GetImGuiLayer()->SetBlockEvents(false);
		
		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (control)
					NewScene();

				break;
			}
			case Key::O:
			{
				if (control)
					OpenScene();

				break;
			}
			case Key::S:
			{
				if (control && shift)
					SaveSceneAs();

				break;
			}
		}
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		return false;
	}

	void EditorLayer::NewScene()
	{
		for (size_t i = 0; i < m_Properties.size(); i++)
			m_Properties[i]->ForceSetContext({});

		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->MarkViewportDirty();
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Arc Scene (*.arc)\0*.arc\0");
		if (!filepath.empty())
		{
			for (size_t i = 0; i < m_Properties.size(); i++)
				m_Properties[i]->ForceSetContext({});

			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->MarkViewportDirty();
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(filepath);
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Arc Scene (*.arc)\0*.arc\0");
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath);
		}
	}

}
