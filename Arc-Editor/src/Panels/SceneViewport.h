#pragma once
#include <ArcEngine.h>

#include "SceneHierarchyPanel.h"

namespace ArcEngine
{
	class SceneViewport
	{
	public:
		SceneViewport();
		virtual ~SceneViewport();

		void OnUpdate(Ref<Scene>& scene, Timestep timestep);
		void OnImGuiRender();

		void SetSceneHierarchyPanel(SceneHierarchyPanel& sceneHierarchyPanel) { m_SceneHierarchyPanel = &sceneHierarchyPanel; }

		bool IsHovered() { return m_ViewportHovered; }
		bool IsFocused() { return m_ViewportFocused; }

	private:
		std::string m_ID;
		Ref<Framebuffer> m_Framebuffer;
		Ref<Framebuffer> m_IDFrameBuffer;
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
