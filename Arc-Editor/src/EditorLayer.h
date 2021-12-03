#pragma once

#include <ArcEngine.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/SceneViewport.h"


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

		Ref<Scene> m_ActiveScene;
		
		float frameTime = 0.0f;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		SceneViewport m_MainSceneViewport;
	};
}
