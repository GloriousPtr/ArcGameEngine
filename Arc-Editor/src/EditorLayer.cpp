#include "EditorLayer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <ArcEngine.h>
#include <Arc/Scene/SceneSerializer.h>
#include <Arc/Scripting/ScriptEngine.h>
#include <Arc/Utils/PlatformUtils.h>
#include <Arc/Math/Math.h>

#include "Utils/EditorTheme.h"
#include "Utils/UI.h"

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

	void EditorLayer::OnAttach()
	{
		ARC_PROFILE_SCOPE();

		EditorTheme::SetFont();
		EditorTheme::SetStyle();
		EditorTheme::ApplyTheme();

		m_Application = &Application::Get();
		m_ActiveScene = CreateRef<Scene>();
		m_EditorScene = m_ActiveScene;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_Viewports.emplace_back(CreateScope<SceneViewport>());
		m_Viewports[0]->SetContext(m_ActiveScene, m_SceneHierarchyPanel);
		
		m_Properties.emplace_back(CreateScope<PropertiesPanel>());
		m_AssetPanels.emplace_back(CreateScope<AssetPanel>());

		m_Panels.emplace_back(CreateScope<RendererSettingsPanel>());
		m_Panels.emplace_back(CreateScope<StatsPanel>());
	}

	void EditorLayer::OnDetach()
	{
		ARC_PROFILE_SCOPE();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		ARC_PROFILE_SCOPE();

		Renderer2D::ResetStats();
		Renderer3D::ResetStats();

		// Remove unused scene viewports
		for (const Scope<SceneViewport>* it = m_Viewports.begin(); it != m_Viewports.end(); it++)
		{
			if (!it->get()->Showing)
			{
				m_Viewports.erase(it);
				break;
			}
		}

		// Remove unused properties panels
		for (const Scope<PropertiesPanel>* it = m_Properties.begin(); it != m_Properties.end(); it++)
		{
			if (!it->get()->Showing)
			{
				m_Properties.erase(it);
				break;
			}
		}

		bool useEditorCamera = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Pause || m_SceneState == SceneState::Step;
		for (const auto& panel : m_Viewports)
		{
			if (panel->Showing)
			{
				panel->OnUpdate(ts);
				panel->SetUseEditorCamera(useEditorCamera);
			}
		}

		for (const auto& panel : m_AssetPanels)
		{
			if (panel->Showing)
				panel->OnUpdate(ts);
		}
		
		for (const auto& panel : m_Panels)
		{
			if (panel->Showing)
				panel->OnUpdate(ts);
		}
	}

	void EditorLayer::HandleWindowDrag()
	{
		if (m_Resizing)
			return;

		Window& window = m_Application->GetWindow();

		ImVec2 mousePosition = ImGui::GetMousePos();
		if (ImGui::IsWindowHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			if (window.IsMaximized())
				window.Restore();
			else
				window.Maximize({mousePosition.x, mousePosition.y});
		}

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))		// Dragging the window
		{
			m_WindowDragging = true;

			if (window.IsMaximized())
			{
				glm::vec2 maximizedPosition = window.GetPosition();
				glm::vec2 maximizedSize = window.GetSize();

				window.Restore();

				float ratio = Math::InverseLerp(maximizedPosition.x, maximizedPosition.x + maximizedSize.x, mousePosition.x);
				float xStart = mousePosition.x - window.GetSize().x * ratio;
				window.SetPosition({ xStart, maximizedPosition.y });
			}
			else
			{
				ImVec2 change = (mousePosition - m_LastMousePosition);
				glm::vec2 windowPos = window.GetPosition();
				glm::vec2 newWindowPos = { change.x + windowPos.x, change.y + windowPos.y };
				window.SetPosition(newWindowPos);
			}
		}
		else if (m_WindowDragging && ImGui::IsItemDeactivated())	// Dropped the window
		{
			m_WindowDragging = false;

			glm::vec4 monitorRect = window.GetMonitorWorkArea({ mousePosition.x, mousePosition.y });
			if (!window.IsMaximized() && mousePosition.y == monitorRect.y)
				window.Maximize({ mousePosition.x, mousePosition.y });
			else
				window.SubmitRestorePosition(window.GetPosition());
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		BeginDockspace("MyDockSpace");
		{
			ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_MenuBar
				| ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			{
				//////////////////////////////////////////////////////////////////////////
				// PRIMARY TOP MENU BAR //////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////
				if (ImGui::BeginViewportSideBar("##PrimaryMenuBar", viewport, ImGuiDir_Up, m_MenuBarHeight, window_flags))
				{
					HandleWindowDrag();

					if (ImGui::BeginMenuBar())
					{
						if (ImGui::BeginMenu("File"))
						{
							if (ImGui::MenuItem("New", "Ctrl+N"))
								NewScene();
							if (ImGui::MenuItem("Open..", "Ctrl+O"))
								OpenScene();
							if (ImGui::MenuItem("Save...", "Ctrl+S"))
								SaveScene();
							if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
								SaveSceneAs();
							if (ImGui::MenuItem("Exit"))
								m_Application->Close();
							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Window"))
						{
							if (ImGui::BeginMenu("Add"))
							{
								if (ImGui::MenuItem("Viewport"))
								{
									size_t index = m_Viewports.size();
									m_Viewports.emplace_back(CreateScope<SceneViewport>());
									m_Viewports[index]->SetContext(m_ActiveScene, m_SceneHierarchyPanel);
								}
								if (ImGui::MenuItem("Properties"))
								{
									m_Properties.emplace_back(CreateScope<PropertiesPanel>());
								}
								if (ImGui::MenuItem("Assets"))
								{
									m_AssetPanels.emplace_back(CreateScope<AssetPanel>());
								}
								ImGui::EndMenu();
							}
							ImGui::MenuItem("Hierarchy", nullptr, &m_ShowSceneHierarchyPanel);

							ImGui::MenuItem(m_ProjectSettingsPanel.GetName(), nullptr, &m_ProjectSettingsPanel.Showing);
							ImGui::MenuItem(m_ConsolePanel.GetName(), nullptr, &m_ConsolePanel.Showing);

							for (const auto& panel : m_Panels)
								ImGui::MenuItem(panel->GetName(), nullptr, &panel->Showing);

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

						if (ImGui::BeginMenu("Scripting"))
						{
							if (ImGui::MenuItem("Reload Assemblies"))
								ScriptEngine::ReloadAppDomain();

							ImGui::EndMenu();
						}

						// Minimize/Maximize/Close buttons
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
						ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });

						ImVec2 region = ImGui::GetContentRegionMax();
						ImVec2 buttonSize = { region.y * 2.0f, region.y };
						ImGui::SetCursorPosX(region.x - 3.0f * buttonSize.x + ImGui::GetStyle().WindowPadding.x);
						bool isNormalCursor = ImGui::GetMouseCursor() == ImGuiMouseCursor_Arrow;

						// Minimize Button
						if (ImGui::Button(ICON_MDI_MINUS, buttonSize) && isNormalCursor)
							m_Application->GetWindow().Minimize();

						// Maximize Button
						if (ImGui::Button(ICON_MDI_WINDOW_MAXIMIZE, buttonSize) && isNormalCursor)
						{
							Window& window = m_Application->GetWindow();
							if (window.IsMaximized())
							{
								window.Restore();
							}
							else
							{
								ImVec2 mousePosition = ImGui::GetMousePos();
								window.Maximize({ mousePosition.x, mousePosition.y });
							}
						}

						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.909f, 0.066f, 0.137f, 1.0f });
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.920f, 0.066f, 0.120f, 1.0f });
						// Close Button
						if (ImGui::Button(ICON_MDI_WINDOW_CLOSE, buttonSize) && isNormalCursor)
							m_Application->Close();
						ImGui::PopStyleColor(2);

						ImGui::PopStyleColor();
						ImGui::PopStyleVar(3);

						ImGui::EndMenuBar();
					}
					ImGui::End();
				}

				//////////////////////////////////////////////////////////////////////////
				// SECONDARY TOP BAR /////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////
				if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, m_MenuBarHeight, window_flags))
				{
					HandleWindowDrag();

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1, 1 });
					if (ImGui::BeginMenuBar())
					{
						ImVec2 region = ImGui::GetContentRegionMax();
						float frameHeight = ImGui::GetFrameHeight();
						ImVec2 buttonSize = { frameHeight * 1.5f, frameHeight };
						ImGui::SetCursorPosX(region.x * 0.5f - 3 * 0.5f * buttonSize.x);

						//Play Button
						bool highlight = m_SceneState == SceneState::Play || m_SceneState == SceneState::Pause || m_SceneState == SceneState::Step;
						const char* icon = m_SceneState == SceneState::Edit ? ICON_MDI_PLAY : ICON_MDI_STOP;
						if (UI::ToggleButton(icon, highlight, buttonSize))
						{
							if (m_SceneState == SceneState::Edit)
								OnScenePlay();
							else
								OnSceneStop();
						}

						//Pause Button
						highlight = m_SceneState == SceneState::Pause || m_SceneState == SceneState::Step;
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, m_SceneState == SceneState::Edit);
						if (UI::ToggleButton(ICON_MDI_PAUSE, highlight, buttonSize))
						{
							if (m_SceneState == SceneState::Play)
								OnScenePause();
							else if (m_SceneState == SceneState::Pause)
								OnSceneUnpause();
						}

						// Step Button
						if (m_SceneState == SceneState::Step)
							OnScenePause();
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, m_SceneState != SceneState::Pause);
						if (ImGui::Button(ICON_MDI_STEP_FORWARD, buttonSize) && m_SceneState == SceneState::Pause)
						{
							OnSceneUnpause();
							m_SceneState = SceneState::Step;
						}

						ImGui::PopItemFlag();
						ImGui::PopItemFlag();

						ImGui::EndMenuBar();
					}
					ImGui::PopStyleVar();
					ImGui::End();
				}

				//////////////////////////////////////////////////////////////////////////
				// BOTTOM MENU BAR ///////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////
				if (ImGui::BeginViewportSideBar("##StatusBar", viewport, ImGuiDir_Down, m_MenuBarHeight, window_flags))
				{
					if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
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
			ImGui::PopStyleVar();

			//////////////////////////////////////////////////////////////////////////
			// HEIRARCHY /////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			if (m_ShowSceneHierarchyPanel)
				m_SceneHierarchyPanel.OnImGuiRender();

			//////////////////////////////////////////////////////////////////////////
			// SCENE VIEWPORTS ///////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			for (const auto& viewportPanel : m_Viewports)
			{
				if (viewportPanel->Showing)
					viewportPanel->OnImGuiRender();
			}

			//////////////////////////////////////////////////////////////////////////
			// ASSETS PANELS /////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			for (const auto& assetPanel : m_AssetPanels)
			{
				if (assetPanel->Showing)
					assetPanel->OnImGuiRender();
			}

			//////////////////////////////////////////////////////////////////////////
			// OTHER PANELS //////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			for (const auto& panel : m_Panels)
			{
				if (panel->Showing)
					panel->OnImGuiRender();
			}

			//////////////////////////////////////////////////////////////////////////
			// PROPERTY PANELS ///////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			for (const auto& propertyPanel : m_Properties)
			{
				if (propertyPanel->Showing)
				{
					propertyPanel->SetContext(m_SelectedContext);
					propertyPanel->OnImGuiRender();
				}
			}

			if (m_ConsolePanel.Showing)
				m_ConsolePanel.OnImGuiRender();

			if (m_ProjectSettingsPanel.Showing)
				m_ProjectSettingsPanel.OnImGuiRender();

			if (m_ShowDemoWindow)
				ImGui::ShowDemoWindow(&m_ShowDemoWindow);

			m_Application->GetImGuiLayer()->SetBlockEvents(false);
		}
		EndDockspace();

		m_LastMousePosition = ImGui::GetMousePos();
		if (m_WindowDragging && !ImGui::IsMouseDown(0))
			m_WindowDragging = false;
	}

	void EditorLayer::OnEvent(Event& e)
	{
		ARC_PROFILE_SCOPE();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnMouseButtonReleased));
	}

	void EditorLayer::BeginDockspace(const char* name)
	{
		HandleResize();

		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoCloseButton;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		if (opt_fullscreen)
		{
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
		m_MenuBarHeight = ImGui::GetFrameHeight();

		float topMenuBarCount = 2.0f;
		float bottomMenuBarCount = 1.0f;
		float dockSpaceOffsetY = ImGui::GetCursorPosY() + (topMenuBarCount - 1.0f) * m_MenuBarHeight;

		ImVec2 dockSpaceSize = ImVec2(viewport->Size.x, viewport->Size.y - ((topMenuBarCount + bottomMenuBarCount) * m_MenuBarHeight));

		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		float minWinSizeY = style.WindowMinSize.y;
		style.WindowMinSize.x = 370.0f;
		style.WindowMinSize.y = 150.0f;
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGui::SetCursorPosY(dockSpaceOffsetY);
			ImGuiID dockspace_id = ImGui::GetID(name);
			ImGui::DockSpace(dockspace_id, dockSpaceSize, dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;
		style.WindowMinSize.y = minWinSizeY;
	}

	void EditorLayer::HandleResize()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		Window& window = m_Application->GetWindow();
		if (!m_WindowDragging && !window.IsMaximized())
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			constexpr float delta = 6.0f;
			bool top = glm::abs(mousePos.y - viewport->Pos.y) <= delta;
			bool left = glm::abs(mousePos.x - viewport->Pos.x) <= delta;
			bool bottom = glm::abs(viewport->Pos.y + viewport->Size.y - mousePos.y) <= delta;
			bool right = glm::abs(viewport->Pos.x + viewport->Size.x - mousePos.x) <= delta;

			if ((top && left) || (bottom && right))
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
			else if ((top && right) || (bottom && left))
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
			else if (top || bottom)
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
			else if (left || right)
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

			if (top)
				m_TopResizing = true;
			if (left)
				m_LeftResizing = true;
			if (bottom)
				m_BottomResizing = true;
			if (right)
				m_RightResizing = true;

			if (m_LeftResizing || m_RightResizing || m_TopResizing || m_BottomResizing)
			{
				if (ImGui::IsMouseClicked(0))
					m_Resizing = true;
				else if (!ImGui::IsMouseDown(0))
					m_Resizing = m_TopResizing = m_LeftResizing = m_BottomResizing = m_RightResizing = false;
			}

			if (ImGui::IsMouseDragging(0) && m_Resizing)
			{

				ImVec2 mousePosition = ImGui::GetMousePos();
				ImVec2 change = mousePosition - m_LastMousePosition;
				glm::vec2 windowPos = window.GetPosition();
				glm::vec2 windowSize = window.GetSize();

				if (m_TopResizing)
				{
					windowSize.y -= change.y;
					windowPos.y += change.y;
				}
				if (m_LeftResizing)
				{
					windowSize.x -= change.x;
					windowPos.x += change.x;
				}
				if (m_BottomResizing)
				{
					windowSize.y += change.y;
				}
				if (m_RightResizing)
				{
					windowSize.x += change.x;
				}

				window.Resize(windowPos, windowSize);
				window.SubmitRestorePosition(windowPos);
				window.SubmitRestoreSize(windowSize);
			}
		}
	}

	void EditorLayer::EndDockspace() const
	{
		ImGui::End();
	}

	bool EditorLayer::OnKeyPressed(const KeyPressedEvent& e)
	{
		ARC_PROFILE_SCOPE();

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
				{
					NewScene();
					return true;
				}
				break;
			}
			case Key::O:
			{
				if (control)
				{
					OpenScene();
					return true;
				}
				break;
			}
			case Key::S:
			{
				if (control && shift)
				{
					SaveSceneAs();
					return true;
				}

				else if (control)
				{
					SaveScene();
					return true;
				}
				break;
			}
			case Key::D:
			{
				if (control && m_SelectedContext.Type == EditorContextType::Entity)
				{
					m_ActiveScene->Duplicate(*((Entity*)m_SelectedContext.Data));
					return true;
				}
				break;
			}
			default:
				break;
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(const MouseButtonPressedEvent& e) const
	{
		return false;
	}

	bool EditorLayer::OnMouseButtonReleased(const MouseButtonReleasedEvent& e) const
	{
		return false;
	}

	void EditorLayer::NewScene()
	{
		ResetContext();
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		for (const auto& propertyPanel : m_Properties)
			propertyPanel->ForceSetContext(m_SelectedContext);

		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->MarkViewportDirty();
		m_EditorScene = m_ActiveScene;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		if (!m_Viewports.empty())
			m_Viewports[0]->SetContext(m_ActiveScene, m_SceneHierarchyPanel);
		m_ScenePath = "";

		ScriptEngine::SetScene(m_ActiveScene.get());
	}

	void EditorLayer::OpenScene(const char* filepath)
	{
		ResetContext();
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		for (const auto& propertyPanel : m_Properties)
			propertyPanel->ForceSetContext(m_SelectedContext);

		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize(1, 1);
		m_ActiveScene->MarkViewportDirty();
		m_EditorScene = m_ActiveScene;
		ScriptEngine::SetScene(m_ActiveScene.get());

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		if (!m_Viewports.empty())
			m_Viewports[0]->SetContext(m_ActiveScene, m_SceneHierarchyPanel);

		SceneSerializer serializer(m_ActiveScene);
		serializer.Deserialize(filepath);
		m_ScenePath = filepath;
	}

	void EditorLayer::OpenScene()
	{
		eastl::string filepath = FileDialogs::OpenFile("Arc Scene (*.arc)\0*.arc\0");
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
			serializer.Serialize(m_ScenePath.string().c_str());
		}
		else
		{
			SaveSceneAs();
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		eastl::string filepath = FileDialogs::SaveFile("Arc Scene (*.arc)\0*.arc\0");
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath);
			m_ScenePath = filepath.c_str();
		}
	}

	void ArcEngine::EditorLayer::OnScenePlay()
	{
		ResetContext();

		m_EditorScene = m_ActiveScene;
		m_RuntimeScene = Scene::CopyTo(m_EditorScene);
		
		m_ActiveScene = m_RuntimeScene;
		ScriptEngine::SetScene(m_ActiveScene.get());
		m_ActiveScene->OnRuntimeStart();
		m_SceneState = SceneState::Play;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		if (m_Viewports.empty())
			m_Viewports.emplace_back(CreateScope<SceneViewport>());
		m_Viewports[0]->SetContext(m_ActiveScene, m_SceneHierarchyPanel);
		m_Viewports[0]->SetSimulation(true);
	}

	void ArcEngine::EditorLayer::OnSceneStop()
	{
		ResetContext();

		m_ActiveScene->OnRuntimeStop();
		m_SceneState = SceneState::Edit;

		m_RuntimeScene = nullptr;
		m_ActiveScene = nullptr;
		m_ActiveScene = m_EditorScene;
		ScriptEngine::SetScene(m_ActiveScene.get());

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		if (!m_Viewports.empty())
		{
			m_Viewports[0]->SetSimulation(false);
			m_Viewports[0]->SetContext(m_ActiveScene, m_SceneHierarchyPanel);
		}
	}

	void ArcEngine::EditorLayer::OnScenePause()
	{
		m_SceneState = SceneState::Pause;

		if (!m_Viewports.empty())
			m_Viewports[0]->SetSimulation(false);
	}

	void ArcEngine::EditorLayer::OnSceneUnpause()
	{
		m_SceneState = SceneState::Play;

		if (!m_Viewports.empty())
			m_Viewports[0]->SetSimulation(true);
	}
}
