#pragma once

#include <ArcEngine.h>

#include "Panels/PropertiesPanel.h"
#include "Panels/SceneViewport.h"
#include "Panels/BasePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/AssetPanel.h"

namespace ArcEngine
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;
		
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

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

		std::vector<Scope<BasePanel>> m_Panels;
		std::vector<Scope<SceneViewport>> m_Viewports;
		std::vector<Scope<PropertiesPanel>> m_Properties;
		std::vector<Scope<AssetPanel>> m_AssetPanels;

		bool m_ShowSceneHierarchyPanel = true;
		bool m_ShowDemoWindow = false;
	};
}
