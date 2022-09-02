#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"
#include "SceneHierarchyPanel.h"

namespace ArcEngine
{
	class SceneViewport : public BasePanel
	{
	public:
		explicit SceneViewport(const char* name = "Viewport");
		virtual ~SceneViewport() override = default;

		SceneViewport(const SceneViewport& other) = delete;
		SceneViewport(SceneViewport&& other) = delete;
		SceneViewport& operator=(const SceneViewport& other) = delete;
		SceneViewport& operator=(SceneViewport&& other) = delete;

		virtual void OnUpdate(const Ref<Scene>& scene, Timestep timestep, bool useEditorCamera);
		virtual void OnImGuiRender() override;

		void OnOverlayRender(const Ref<Scene>& scene) const;

		bool OnMouseButtonPressed(const MouseButtonPressedEvent& e);
		bool OnMouseButtonReleased(const MouseButtonReleasedEvent& e);
		void SetSceneHierarchyPanel(SceneHierarchyPanel& sceneHierarchyPanel) { m_SceneHierarchyPanel = &sceneHierarchyPanel; }

		bool IsHovered() const { return m_ViewportHovered; }
		bool IsFocused() const { return m_ViewportFocused; }
		
		void SetSimulation(const bool value) { m_SimulationRunning = value; }

	private:
		void OnInit();

	private:
		Ref<RenderGraphData> m_RenderGraphData;
		EditorCamera m_EditorCamera;
		Ref<Scene> m_Scene;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];

		int m_GizmoType = -1;

		SceneHierarchyPanel* m_SceneHierarchyPanel = nullptr;

		glm::vec2 m_MousePosition = glm::vec2(0.0f);
		glm::vec2 m_LastMousePosition = glm::vec2(0.0f);
		float m_MouseSensitivity = 0.1f;

		float m_MaxMoveSpeed = 1.0f;
		float m_MoveDampeningFactor = 0.000001f;
		glm::vec3 m_MoveDirection = glm::vec3(0.0f);
		float m_MoveVelocity = 0.0f;

		bool m_SimulationRunning = false;
		bool m_CursorLocked = false;
	};
}
