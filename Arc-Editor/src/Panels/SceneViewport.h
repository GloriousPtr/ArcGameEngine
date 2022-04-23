#pragma once
#include <ArcEngine.h>

#include "BasePanel.h"
#include "SceneHierarchyPanel.h"

namespace ArcEngine
{
	class SceneViewport : public BasePanel
	{
	public:
		SceneViewport();
		SceneViewport(const char* name);
		virtual ~SceneViewport() override = default;

		virtual void OnUpdate(Ref<Scene>& scene, Timestep timestep);
		virtual void OnImGuiRender() override;

		void SetSceneHierarchyPanel(SceneHierarchyPanel& sceneHierarchyPanel) { m_SceneHierarchyPanel = &sceneHierarchyPanel; }

		bool IsHovered() { return m_ViewportHovered; }
		bool IsFocused() { return m_ViewportFocused; }

	private:
		void OnInit();

	private:
		std::string m_ID;
		Ref<RenderGraphData> m_RenderGraphData;
		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];

		int m_GizmoType = -1;

		SceneHierarchyPanel* m_SceneHierarchyPanel = nullptr;

		glm::vec2 m_MousePosition;
		glm::vec2 m_LastMousePosition;
		float m_MovementSpeed = 5.0f;
		float m_MouseSensitivity = 0.1f;
	};
}
