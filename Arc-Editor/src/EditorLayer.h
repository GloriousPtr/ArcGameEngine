#pragma once

#include <ArcEngine.h>

#include "Panels/BasePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/SceneViewport.h"
#include "Panels/PropertiesPanel.h"

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
		void SaveSceneAs();
	private:

		Application* m_Application;
		Ref<Scene> m_ActiveScene;
		
		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;

		std::vector<Scope<BasePanel>> m_Panels;
		std::vector<Scope<SceneViewport>> m_Viewports;
		std::vector<Scope<PropertiesPanel>> m_Properties;

		bool m_ShowSceneHierarchyPanel = true;
		bool m_ShowConsole = true;
	};
}
