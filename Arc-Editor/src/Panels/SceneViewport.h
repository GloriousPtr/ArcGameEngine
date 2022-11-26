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
		int m_GizmoMode = 0;

		SceneHierarchyPanel* m_SceneHierarchyPanel = nullptr;

		glm::vec2 m_LockedMousePosition = glm::vec2(0.0f);
		float m_MouseSensitivity = 0.06f;
		float m_MaxMoveSpeed = 10.0f;
		float m_TranslationDampening = 0.6f;
		float m_RotationDampening = 0.3f;
		glm::vec3 m_TranslationVelocity = glm::vec3(0.0f);
		glm::vec2 m_RotationVelocity = glm::vec2(0.0f);
		glm::mat4 m_CubeViewMatrix = glm::mat4(1.0f);
		glm::vec2 m_GizmoPosition = glm::vec2(1.0f);

		bool m_UseEditorCamera = true;
		bool m_UsingEditorCamera = false;
		bool m_SimulationRunning = false;
	};
}
