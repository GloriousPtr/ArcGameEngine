#include "EditorLayer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ArcEngine.h>

#include "Utils/EditorTheme.h"

#include "Arc/Scene/SceneSerializer.h"
#include "Arc/Utils/PlatformUtils.h"
#include "Arc/Math/Math.h"

#include "Panels/RendererSettingsPanel.h"
#include "Panels/StatsPanel.h"

namespace ArcEngine
{
	EditorLayer* EditorLayer::s_Instance = nullptr;

	EditorLayer::EditorLayer()
		: Layer("Sandbox2D")
	{
		ARC_CORE_ASSERT(!s_Instance, "Editor Layer already exists!");

		s_Instance = this;
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

		EditorTheme::SetFont();
		EditorTheme::SetStyle();
		EditorTheme::ApplyTheme();

		m_Application = &Application::Get();
		m_ActiveScene = CreateRef<Scene>();
		m_EditorScene = m_ActiveScene;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_Viewports.push_back(CreateScope<SceneViewport>());
		m_Viewports[0]->SetSceneHierarchyPanel(m_SceneHierarchyPanel);
		
		m_Properties.push_back(CreateScope<PropertiesPanel>());
		m_AssetPanels.push_back(CreateScope<AssetPanel>());

		m_Panels.push_back(CreateScope<RendererSettingsPanel>());
		m_Panels.push_back(CreateScope<StatsPanel>());
	}

	void EditorLayer::OnDetach()
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();
		
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

		Renderer2D::ResetStats();
		Renderer3D::ResetStats();

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
		OPTICK_EVENT();

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
		float menuBarHeight = ImGui::GetFrameHeight();
		uint32_t topMenuBarCount = 2;
		uint32_t bottomMenuBarCount = 1;
		float dockSpaceOffsetY = ImGui::GetCursorPosY() + (topMenuBarCount - 1) * menuBarHeight;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 dockSpaceSize = ImVec2(viewport->Size.x, viewport->Size.y - (topMenuBarCount + bottomMenuBarCount) * menuBarHeight);

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		float minWinSizeY = style.WindowMinSize.y;
		style.WindowMinSize.x = 370.0f;
		style.WindowMinSize.y = 50.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGui::SetCursorPosY(dockSpaceOffsetY);
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, dockSpaceSize, dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;
		style.WindowMinSize.y = minWinSizeY;
		{
			ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

			//////////////////////////////////////////////////////////////////////////
			// PRIMARY TOP MENU BAR //////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			if (ImGui::BeginViewportSideBar("##PrimaryMenuBar", viewport, ImGuiDir_Up, menuBarHeight, window_flags))
			{
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
						if (ImGui::MenuItem("Save...", "Ctrl+S"))
						{
							SaveScene();
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
							if (ImGui::MenuItem("Assets"))
							{
								m_AssetPanels.push_back(CreateScope<AssetPanel>());
							}
							ImGui::EndMenu();
						}
						ImGui::MenuItem("Hierarchy", nullptr, &m_ShowSceneHierarchyPanel);

						{
							bool showing = m_ConsolePanel.IsShowing();
							if (ImGui::MenuItem(m_ConsolePanel.GetName(), nullptr, &showing))
								m_ConsolePanel.SetShowing(showing);
						}
						for (size_t i = 0; i < m_Panels.size(); i++)
						{
							BasePanel* panel = m_Panels[i].get();
							bool showing = panel->IsShowing();
							if (ImGui::MenuItem(panel->GetName(), nullptr, &showing))
								panel->SetShowing(showing);
						}

						ImGui::MenuItem("ImGui Demo Window", nullptr, &m_ShowDemoWindow);

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
				ImGui::End();
			}

			//////////////////////////////////////////////////////////////////////////
			// SECONDARY TOP BAR /////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, menuBarHeight, window_flags))
			{
				if (ImGui::BeginMenuBar())
				{
					ImVec2 region = ImGui::GetContentRegionAvail();
					ImGui::SetCursorPosX(region.x * 0.5f);
					if (ImGui::Button(m_SceneState == SceneState::Edit ? ICON_MDI_PLAY : ICON_MDI_STOP))
					{
						if (m_SceneState == SceneState::Edit)
							OnScenePlay();
						else
							OnSceneStop();
					}

					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, m_SceneState == SceneState::Edit);
					if (ImGui::Button(ICON_MDI_PAUSE))
					{
						OnScenePause();
					}
					if (ImGui::Button(ICON_MDI_STEP_FORWARD))
					{
					}
					ImGui::PopItemFlag();

					ImGui::EndMenuBar();
				}
				ImGui::End();
			}

			//////////////////////////////////////////////////////////////////////////
			// BOTTOM MENU BAR ///////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			if (ImGui::BeginViewportSideBar("##StatusBar", viewport, ImGuiDir_Down, menuBarHeight, window_flags))
			{
				if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
					m_ConsolePanel.SetFocus();

				if (ImGui::BeginMenuBar())
				{
					const ConsolePanel::Message* message = m_ConsolePanel.GetRecentMessage();
					if (message != nullptr)
					{
						glm::vec4 color = ConsolePanel::Message::GetRenderColor(message->Level);
						ImGui::PushStyleColor(ImGuiCol_Text, { color.r, color.g, color.b, color.a });
						ImGui::TextUnformatted(message->Buffer.c_str());
						ImGui::PopStyleColor();
					}
					ImGui::EndMenuBar();
				}

				ImGui::End();
			}
		}
		
		//////////////////////////////////////////////////////////////////////////
		// HEIRARCHY /////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		if (m_ShowSceneHierarchyPanel)
			m_SceneHierarchyPanel.OnImGuiRender();
		Entity selectedContext = m_SceneHierarchyPanel.GetSelectedEntity();

		//////////////////////////////////////////////////////////////////////////
		// SCENE VIEWPORTS ///////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		for (size_t i = 0; i < m_Viewports.size(); i++)
		{
			if (m_Viewports[i]->IsShowing())
				m_Viewports[i]->OnImGuiRender();
		}

		//////////////////////////////////////////////////////////////////////////
		// ASSETS PANELS /////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		for (size_t i = 0; i < m_AssetPanels.size(); i++)
		{
			if (m_AssetPanels[i]->IsShowing())
				m_AssetPanels[i]->OnImGuiRender();
		}

		//////////////////////////////////////////////////////////////////////////
		// OTHER PANELS //////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		for (size_t i = 0; i < m_Panels.size(); i++)
		{
			if (m_Panels[i]->IsShowing())
				m_Panels[i]->OnImGuiRender();
		}

		//////////////////////////////////////////////////////////////////////////
		// PROPERTY PANELS ///////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		for (size_t i = 0; i < m_Properties.size(); i++)
		{
			if (m_Properties[i]->IsShowing())
			{
				m_Properties[i]->SetContext(selectedContext);
				m_Properties[i]->OnImGuiRender();
			}
		}

		if (m_ConsolePanel.IsShowing())
			m_ConsolePanel.OnImGuiRender();

		if (m_ProjectSettingsPanel.IsShowing())
			m_ProjectSettingsPanel.OnImGuiRender();

		if (m_ShowDemoWindow)
			ImGui::ShowDemoWindow(&m_ShowDemoWindow);

		m_Application->GetImGuiLayer()->SetBlockEvents(false);
		
		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		OPTICK_EVENT();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnMouseButtonReleased));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		OPTICK_EVENT();

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
				else if (control)
					SaveScene();
				break;
			}
		}
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		bool handled = false;
		for (auto& viewport : m_Viewports)
		{
			handled = viewport->OnMouseButtonPressed(e);
			if (handled)
				return true;
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
	{
		bool handled = false;
		for (auto& viewport : m_Viewports)
		{
			handled = viewport->OnMouseButtonReleased(e);
			if (handled)
				return true;
		}

		return false;
	}

	void EditorLayer::NewScene()
	{
		for (size_t i = 0; i < m_Properties.size(); i++)
			m_Properties[i]->ForceSetContext({});

		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->MarkViewportDirty();
		m_EditorScene = m_ActiveScene;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_ScenePath = "";
	}

	void EditorLayer::OpenScene(const char* filepath)
	{
		for (size_t i = 0; i < m_Properties.size(); i++)
			m_Properties[i]->ForceSetContext({ entt::null , nullptr });

		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize(1, 1);
		m_ActiveScene->MarkViewportDirty();
		m_EditorScene = m_ActiveScene;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		SceneSerializer serializer(m_ActiveScene);
		serializer.Deserialize(filepath);
		m_ScenePath = filepath;
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Arc Scene (*.arc)\0*.arc\0");
		if (!filepath.empty())
		{
			OpenScene(filepath.c_str());
		}
	}
	
	void EditorLayer::SaveScene()
	{
		if (!m_ScenePath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(m_ScenePath.string());
		}
		else
		{
			SaveSceneAs();
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Arc Scene (*.arc)\0*.arc\0");
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath);
			m_ScenePath = filepath;
		}
	}

	void ArcEngine::EditorLayer::OnScenePlay()
	{
		m_EditorScene = m_ActiveScene;
		m_RuntimeScene = Scene::CopyTo(m_EditorScene);
		
		m_ActiveScene = m_RuntimeScene;
		m_ActiveScene->OnRuntimeStart();
		m_SceneState = SceneState::Play;

		if (m_Viewports.size() < 0)
			m_Viewports.push_back(CreateScope<SceneViewport>());

		m_Viewports[0]->SetSceneHierarchyPanel(m_SceneHierarchyPanel);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_Viewports[0]->SetSimulation(true);
	}

	void ArcEngine::EditorLayer::OnSceneStop()
	{
		m_ActiveScene->OnRuntimeStop();
		m_SceneState = SceneState::Edit;

		m_RuntimeScene = nullptr;
		m_ActiveScene = nullptr;
		m_ActiveScene = m_EditorScene;

		if (m_Viewports.size() > 0)
			m_Viewports[0]->SetSimulation(false);

		m_Viewports[0]->SetSceneHierarchyPanel(m_SceneHierarchyPanel);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void ArcEngine::EditorLayer::OnScenePause()
	{
		m_SceneState = SceneState::Pause;

		if (m_Viewports.size() > 0)
			m_Viewports[0]->SetSimulation(false);
	}
}
