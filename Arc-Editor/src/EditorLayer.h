#pragma once

#include <ArcEngine.h>

#include "Panels/PropertiesPanel.h"
#include "Panels/SceneViewport.h"
#include "Panels/BasePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/AssetPanel.h"
#include "Panels/ProjectSettingsPanel.h"

namespace ArcEngine
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer();
		
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
		void OpenScene(const char* filepath);

		static EditorLayer* GetInstance() { return s_Instance; }

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);

		void NewScene();
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
	private:

		Application* m_Application;
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		enum class SceneState { Edit, Play, Pause };
		SceneState m_SceneState = SceneState::Edit;
		std::filesystem::path m_ScenePath = std::filesystem::path();

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ConsolePanel m_ConsolePanel;
		ProjectSettingsPanel m_ProjectSettingsPanel;

		eastl::vector<Scope<BasePanel>> m_Panels;
		eastl::vector<Scope<SceneViewport>> m_Viewports;
		eastl::vector<Scope<PropertiesPanel>> m_Properties;
		eastl::vector<Scope<AssetPanel>> m_AssetPanels;

		bool m_ShowSceneHierarchyPanel = true;
		bool m_ShowDemoWindow = false;

		static EditorLayer* s_Instance;
	};
}
