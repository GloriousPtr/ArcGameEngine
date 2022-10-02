#pragma once

#include <ArcEngine.h>
#include <Arc/Renderer/EditorCamera.h>

#include "BasePanel.h"

namespace ArcEngine
{
	class SceneHierarchyPanel;

	class SceneViewport : public BasePanel
	{
	public:
		explicit SceneViewport(const char* name = "Viewport");
		virtual ~SceneViewport() override = default;

		SceneViewport(const SceneViewport& other) = delete;
		SceneViewport(SceneViewport&& other) = delete;
		SceneViewport& operator=(const SceneViewport& other) = delete;
		SceneViewport& operator=(SceneViewport&& other) = delete;

		virtual void OnUpdate([[maybe_unused]] Timestep timestep) override;
		virtual void OnImGuiRender() override;

		void OnOverlayRender() const;

		bool IsHovered() const { return m_ViewportHovered; }
		bool IsFocused() const { return m_ViewportFocused; }
		
		void SetContext(const Ref<Scene>& scene, SceneHierarchyPanel& sceneHierarchyPanel) { m_SceneHierarchyPanel = &sceneHierarchyPanel; m_Scene = scene; }
		void SetUseEditorCamera(bool state) { m_UseEditorCamera = state; }
		void SetSimulation(const bool value) { m_SimulationRunning = value; }

	private:
		void OnInit();

	private:
		Ref<Scene> m_Scene;
		Ref<RenderGraphData> m_RenderGraphData = nullptr;
		Ref<RenderGraphData> m_MiniViewportRenderGraphData = nullptr;
		float m_MiniViewportSizeMultiplier = 0.25f;
		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];

		int m_GizmoType = -1;

		SceneHierarchyPanel* m_SceneHierarchyPanel = nullptr;

		glm::vec2 m_MousePosition = glm::vec2(0.0f);
		float m_MouseSensitivity = 0.1f;

		float m_MaxMoveSpeed = 2.0f;
		float m_MoveDampeningFactor = 0.000001f;
		glm::vec3 m_MoveDirection = glm::vec3(0.0f);
		float m_MoveVelocity = 0.0f;

		bool m_UseEditorCamera = true;
		bool m_UsingEditorCamera = false;
		bool m_SimulationRunning = false;
	};
}
