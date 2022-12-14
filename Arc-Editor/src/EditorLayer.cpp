#include "EditorLayer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <ArcEngine.h>
#include <Arc/Project/ProjectSerializer.h>
#include <Arc/Scene/SceneSerializer.h>
#include <Arc/Scripting/ScriptEngine.h>
#include <Arc/Utils/PlatformUtils.h>
#include <Arc/Math/Math.h>
#include <icons/IconsMaterialDesignIcons.h>

#include "Panels/AssetPanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/RendererSettingsPanel.h"
#include "Panels/SceneViewport.h"
#include "Panels/StatsPanel.h"

#include "Utils/EditorTheme.h"
#include "Utils/UI.h"

namespace ArcEngine
{
	EditorLayer* EditorLayer::s_Instance = nullptr;

	EditorLayer::EditorLayer()
		: Layer("Arc-Editor")
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

		if (Project::GetActive().get())
			m_AssetPanels.emplace_back(CreateScope<AssetPanel>());

		m_Panels.emplace_back(CreateScope<ProjectSettingsPanel>());
		m_Panels.emplace_back(CreateScope<RendererSettingsPanel>());
		m_Panels.emplace_back(CreateScope<StatsPanel>());

		OpenProject();
	}

	void EditorLayer::OnDetach()
	{
		ARC_PROFILE_SCOPE();
	}

	void EditorLayer::OnUpdate([[maybe_unused]] Timestep ts)
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

		// Remove unused asset panels
		for (const Scope<AssetPanel>* it = m_AssetPanels.begin(); it != m_AssetPanels.end(); it++)
		{
			if (!it->get()->Showing)
			{
				m_AssetPanels.erase(it);
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

	void EditorLayer::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		m_Application->GetWindow().RegisterOverTitlebar(false);

		BeginDockspace("MyDockSpace");
		{
			auto* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_MenuBar
				| ImGuiWindowFlags_NoNavFocus;

			ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;
			float frameHeight = ImGui::GetFrameHeight();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			{
				//////////////////////////////////////////////////////////////////////////
				// PRIMARY TOP MENU BAR //////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////
				ImVec2 framePadding = ImGui::GetStyle().FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { framePadding.x, 6.5f });
				m_TopMenuBarHeight = ImGui::GetFrameHeight();
				if (ImGui::BeginViewportSideBar("##PrimaryMenuBar", viewport, ImGuiDir_Up, m_TopMenuBarHeight, window_flags))
				{
					if (ImGui::BeginMenuBar())
					{
						ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);

						if (ImGui::BeginMenu("File"))
						{
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);

							if (ImGui::MenuItem("New Scene", "Ctrl+N"))
								NewScene();
							if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
								SaveScene();
							if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
								SaveSceneAs();
							ImGui::Separator();
							if (ImGui::MenuItem("New Project", "Ctrl+Alt+N"))
								m_ShowNewProjectModal = true;
							if (ImGui::MenuItem("Open Project", "Ctrl+Alt+O"))
								OpenProject();
							ImGui::Separator();
							if (ImGui::BeginMenu("Theme"))
							{
								if (ImGui::MenuItem("Light"))
									EditorTheme::ApplyTheme(false);
								if (ImGui::MenuItem("Dark"))
									EditorTheme::ApplyTheme(true);

								ImGui::EndMenu();
							}
							if (ImGui::MenuItem("Exit"))
								m_Application->Close();

							ImGui::PopStyleVar();
							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Window"))
						{
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);

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
								ImGui::BeginDisabled(!Project::GetActive().get());
								if (ImGui::MenuItem("Assets"))
								{
									m_AssetPanels.emplace_back(CreateScope<AssetPanel>());
								}
								ImGui::EndDisabled();

								ImGui::EndMenu();
							}
							ImGui::Separator();
							ImGui::MenuItem("Hierarchy", nullptr, &m_ShowSceneHierarchyPanel);
							ImGui::MenuItem(m_ConsolePanel.GetName(), nullptr, &m_ConsolePanel.Showing);

							for (const auto& panel : m_Panels)
								ImGui::MenuItem(panel->GetName(), nullptr, &panel->Showing);

							ImGui::Separator();
							ImGui::MenuItem("ImGui Demo Window", nullptr, &m_ShowDemoWindow);

							ImGui::PopStyleVar();
							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Shaders"))
						{
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);

							if (ImGui::MenuItem("Reload Shaders"))
							{
								Renderer3D::GetShaderLibrary().ReloadAll();
								Renderer3D::Init();
							}

							ImGui::PopStyleVar();
							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Scripting"))
						{
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);

							if (ImGui::MenuItem("Reload Assemblies"))
								ScriptEngine::ReloadAppDomain();

							ImGui::PopStyleVar();
							ImGui::EndMenu();
						}

						ImGui::PopStyleVar();

						ImVec2 region = ImGui::GetContentRegionMax();
						ImVec2 buttonSize = { region.y * 1.6f, region.y };

						ImVec2 windowGrabAreaStart = ImGui::GetCursorPos();
						float buttonStartRegion = region.x - 3.0f * buttonSize.x + ImGui::GetStyle().WindowPadding.x;
						glm::vec4 windowGrabArea = glm::vec4(windowGrabAreaStart.x, windowGrabAreaStart.y - windowPadding.y, buttonStartRegion, windowGrabAreaStart.y + frameHeight + windowPadding.y);
						ImGui::InvisibleButton("TitlebarGrab1", { buttonStartRegion - windowGrabAreaStart.x, frameHeight + windowPadding.y });
						if (ImGui::IsItemHovered())
							m_Application->GetWindow().RegisterOverTitlebar(true);

						ImGui::PushStyleColor(ImGuiCol_Button, EditorTheme::WindowBgAlternativeColor);
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::WindowBgAlternativeColor);
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorTheme::WindowBgAlternativeColor);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						if (auto project = Project::GetActive())
						{
							const eastl::string& projectName = project->GetConfig().Name;
							ImVec2 textSize = ImGui::CalcTextSize(projectName.c_str());
							ImGui::SetCursorPos(ImVec2(region.x - 4.0f * buttonSize.x - textSize.x - 100.0f + ImGui::GetStyle().WindowPadding.x, ImGui::GetCursorPosY() - 2.0f));
							ImGui::Button(projectName.c_str(), { textSize.x + 100.0f, buttonSize.y });
						}
						ImGui::PopStyleVar();
						ImGui::PopStyleColor(3);

						// Minimize/Maximize/Close buttons
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
						
						ImGui::SetCursorPosX(buttonStartRegion);
						bool isNormalCursor = ImGui::GetMouseCursor() == ImGuiMouseCursor_Arrow;

						// Minimize Button
						if (ImGui::Button(ICON_MDI_MINUS, buttonSize) && isNormalCursor)
							m_Application->GetWindow().Minimize();

						// Maximize Button
						if (ImGui::Button(ICON_MDI_WINDOW_MAXIMIZE, buttonSize) && isNormalCursor)
						{
							Window& window = m_Application->GetWindow();
							if (window.IsMaximized())
								window.Restore();
							else
								window.Maximize();
						}

						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.909f, 0.066f, 0.137f, 1.0f });
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.920f, 0.066f, 0.120f, 1.0f });
						// Close Button
						if (ImGui::Button(ICON_MDI_WINDOW_CLOSE, buttonSize) && isNormalCursor)
							m_Application->Close();
						ImGui::PopStyleColor(2);

						ImGui::PopStyleColor();
						ImGui::PopStyleVar(4);

						ImGui::EndMenuBar();
					}

					ImGui::End();
				}
				ImGui::PopStyleVar();

				//////////////////////////////////////////////////////////////////////////
				// SECONDARY TOP BAR /////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////
				if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, frameHeight, window_flags))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1, 1 });
					if (ImGui::BeginMenuBar())
					{
						ImVec2 region = ImGui::GetContentRegionMax();

						ImVec2 buttonSize = { frameHeight * 1.5f, frameHeight };
						constexpr uint8_t buttonCount = 3;
						float buttonStartPositionX = region.x * 0.5f - buttonCount * 0.5f * buttonSize.x;

						ImGui::InvisibleButton("TitlebarGrab2", { buttonStartPositionX - buttonSize.x * 0.25f, region.y });
						if (ImGui::IsItemHovered())
							m_Application->GetWindow().RegisterOverTitlebar(true);
						ImGui::SetItemAllowOverlap();

						ImGui::SetCursorPosX(buttonStartPositionX);
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

						ImGui::InvisibleButton("TitlebarGrab3", region);
						if (ImGui::IsItemHovered())
							m_Application->GetWindow().RegisterOverTitlebar(true);
						ImGui::SetItemAllowOverlap();

						ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize("FPS: XX.XX (XX.XXXms  MEM: XXXX.XXMB").x);
						float fps = ImGui::GetIO().Framerate;
						size_t allocatedMemory = Application::GetAllocatedMemorySize();
						ImGui::Text("FPS: %.2f (%.3fms)  MEM: %.2fMB", fps, 1000.0f / fps, (float)allocatedMemory / (1024.0f * 1024.0f));
						
						ImGui::EndMenuBar();
					}
					ImGui::PopStyleVar(2);
					ImGui::End();
				}

				//////////////////////////////////////////////////////////////////////////
				// BOTTOM MENU BAR ///////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////
				if (ImGui::BeginViewportSideBar("##StatusBar", viewport, ImGuiDir_Down, frameHeight, window_flags))
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
							ImGui::PushFont(EditorTheme::BoldFont);
							ImGui::TextUnformatted(message->Buffer.c_str());
							ImGui::PopFont();
							ImGui::PopStyleColor();
						}
						ImGui::EndMenuBar();
					}

					ImGui::End();
				}
			}
			ImGui::PopStyleVar(2);
			
			//////////////////////////////////////////////////////////////////////////
			// HIERARCHY /////////////////////////////////////////////////////////////
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

			if (m_ShowNewProjectModal)
			{
				m_ShowNewProjectModal = false;
				ImGui::OpenPopup("New Project");
			}

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				static eastl::string prjName = "";
				static eastl::string folderPath = "";

				constexpr size_t size = 256;
				char buffer[size];
				memcpy(buffer, prjName.data(), size);
				ImGui::TextUnformatted("Project Name");
				if (ImGui::InputText("##ProjectName", buffer, size))
					prjName = buffer;

				memcpy(buffer, folderPath.data(), size);
				ImGui::TextUnformatted("Location");
				if (ImGui::InputText("##ProjectLocation", buffer, size))
					folderPath = buffer;
				ImGui::SameLine();
				if (ImGui::Button(ICON_MDI_FOLDER))
					folderPath = FileDialogs::OpenFolder().c_str();

				ImGui::Separator();

				ImGui::SetItemDefaultFocus();
				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					if (!prjName.empty() && !folderPath.empty())
					{
						Project::New();
						Project::GetActive()->GetConfig().Name = prjName.c_str();
						std::filesystem::path projectDirPath = std::filesystem::path(folderPath.c_str()) / prjName.c_str();
						if (!std::filesystem::exists(projectDirPath))
							std::filesystem::create_directories(projectDirPath);

						std::filesystem::path projectPath = projectDirPath / (prjName + ".arcproj").c_str();
						SaveProject(projectPath);
						OpenProject(projectPath);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (m_ShowDemoWindow)
				ImGui::ShowDemoWindow(&m_ShowDemoWindow);

			m_Application->GetImGuiLayer()->SetBlockEvents(false);
		}
		EndDockspace();

		ImVec2 mousePosition = ImGui::GetMousePos();
		m_LastMousePosition = { mousePosition.x, mousePosition.y };
	}

	void EditorLayer::OnEvent([[maybe_unused]] Event& e)
	{
		ARC_PROFILE_SCOPE();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(ARC_BIND_EVENT_FN(EditorLayer::OnMouseButtonReleased));
	}

	void EditorLayer::BeginDockspace(const char* name) const
	{
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

		float frameHeight = ImGui::GetFrameHeight();
		float menuBarSize = 1.0f * frameHeight;
		float bottomMenuBarSize = 1.0f * frameHeight;
		float dockSpaceOffsetY = ImGui::GetCursorPosY() + menuBarSize + (m_TopMenuBarHeight - frameHeight);

		ImVec2 dockSpaceSize = ImVec2(viewport->Size.x, viewport->Size.y - (menuBarSize + bottomMenuBarSize + m_TopMenuBarHeight));

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

	void EditorLayer::EndDockspace() const
	{
		ImGui::End();
	}

	bool EditorLayer::OnKeyPressed([[maybe_unused]] const KeyPressedEvent& e)
	{
		ARC_PROFILE_SCOPE();

		/*
		HotKeys:
			Entity:
				Ctrl+Shift+N		New empty entity
				Alt+Shift+N			New empty child to selected entity

			Edit:
				Ctrl+D				Duplicate
				Del					Delete
				Ctrl+P				Play
				Ctrl+Shift+P		Pause
				Ctrl+Alt+P			Step

			Scene:
				Ctrl+N				Load new scene
				Ctrl+S				Save current scene
				Ctrl+Shift+S		Save current scene as
				Ctrl+Alt+N			New project
				Ctrl+Alt+O			Open project
		*/

		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		switch (e.GetKeyCode())
		{
			case Key::D:
			{
				if (ctrl && m_SelectedContext.GetType() == EditorContextType::Entity)
				{
					m_ActiveScene->Duplicate(*m_SelectedContext.As<Entity>());
					return true;
				}
				break;
			}
			case Key::P:
			{
				if (ctrl && alt && m_SceneState == SceneState::Pause)
				{
					OnSceneUnpause();
					m_SceneState = SceneState::Step;
					return true;
				}
				break;
			}
		}

		if (e.GetRepeatCount() > 0)
			return false;

		switch (e.GetKeyCode())
		{
			case Key::P:
			{
				if (ctrl && shift)
				{
					if (m_SceneState == SceneState::Edit)
					{
						OnScenePlay();
						OnScenePause();
						return true;
					}
					if (m_SceneState == SceneState::Play)
					{
						OnScenePause();
						return true;
					}
					if (m_SceneState == SceneState::Pause)
					{
						OnSceneUnpause();
						return true;
					}
				}
				if (ctrl)
				{
					if (m_SceneState == SceneState::Edit)
					{
						OnScenePlay();
						return true;
					}
					
					OnSceneStop();
					return true;
				}
				break;
			}
			case Key::N:
			{
				if (ctrl && alt)
				{
					m_ShowNewProjectModal = true;
					return true;
				}
				if (ctrl && shift)
				{
					Entity entity = m_ActiveScene->CreateEntity();
					m_SelectedContext.Set(EditorContextType::Entity, &entity, sizeof(Entity));
					return true;
				}
				if (ctrl)
				{
					NewScene();
					return true;
				}
				if (alt && shift && m_SelectedContext.GetType() == EditorContextType::Entity)
				{
					Entity child = m_ActiveScene->CreateEntity();
					child.SetParent(*m_SelectedContext.As<Entity>());
					m_SelectedContext.Set(EditorContextType::Entity, &child, sizeof(Entity));
					return true;
				}
				break;
			}
			case Key::O:
			{
				if (ctrl && alt)
				{
					OpenProject();
					return true;
				}
				break;
			}
			case Key::S:
			{
				if (ctrl && shift)
				{
					SaveSceneAs();
					return true;
				}
				if (ctrl)
				{
					SaveScene();
					return true;
				}
				break;
			}
			case Key::Delete:
			{
				if (GImGui->ActiveId == 0 && m_SelectedContext.GetType() == EditorContextType::Entity)
				{
					m_ActiveScene->DestroyEntity(*m_SelectedContext.As<Entity>());
					m_SelectedContext.Reset();
					return true;
				}
				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed([[maybe_unused]] const MouseButtonPressedEvent& e) const
	{
		return false;
	}

	bool EditorLayer::OnMouseButtonReleased([[maybe_unused]] const MouseButtonReleasedEvent& e) const
	{
		return false;
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{
			ScriptEngine::ReloadAppDomain();

			if (!Project::GetActive()->GetConfig().StartScene.empty())
			{
				auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
				OpenScene(startScenePath.string().c_str());
			}

			if (m_AssetPanels.empty())
			{
				m_AssetPanels.push_back(CreateScope<AssetPanel>());
			}
			else
			{
				for (const auto& assetPanel : m_AssetPanels)
					assetPanel->Invalidate();
			}
		}
	}

	void EditorLayer::OpenProject()
	{
		eastl::string filepath = FileDialogs::OpenFile("Arc Scene (*.arcproj)\0*.arcproj\0");
		if (!filepath.empty())
			OpenProject(filepath.c_str());
	}

	void EditorLayer::SaveProject(const std::filesystem::path& path) const
	{
		Project::SaveActive(path);
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

	void EditorLayer::OnScenePlay()
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

	void EditorLayer::OnSceneStop()
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

	void EditorLayer::OnScenePause()
	{
		m_SceneState = SceneState::Pause;

		if (!m_Viewports.empty())
			m_Viewports[0]->SetSimulation(false);
	}

	void EditorLayer::OnSceneUnpause()
	{
		m_SceneState = SceneState::Play;

		if (!m_Viewports.empty())
			m_Viewports[0]->SetSimulation(true);
	}
}
