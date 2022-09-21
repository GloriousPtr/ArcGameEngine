#pragma once

#include <ArcEngine.h>

#include "Panels/EditorContext.h"
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
		virtual ~EditorLayer() override = default;
		
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate([[maybe_unused]] Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent([[maybe_unused]] Event& e) override;

		void SetContext(EditorContextType type, const void* data, size_t size) { m_SelectedContext.Set(type, data, size); }
		void ResetContext() { m_SelectedContext.Reset(); }
		const EditorContext& GetContext() const { return m_SelectedContext; }
		
		void OpenScene(const char* filepath);

		static EditorLayer* GetInstance() { return s_Instance; }

	private:
		void HandleWindowDrag();
		void HandleResize();
		void BeginDockspace(const char* name);
		void EndDockspace() const;
		bool OnKeyPressed([[maybe_unused]] const KeyPressedEvent& e);
		bool OnMouseButtonPressed([[maybe_unused]] const MouseButtonPressedEvent& e) const;
		bool OnMouseButtonReleased([[maybe_unused]] const MouseButtonReleasedEvent& e) const;

		void NewScene();
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void OnSceneUnpause();
	private:

		Application* m_Application;
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		enum class SceneState { Edit, Play, Pause, Step };
		SceneState m_SceneState = SceneState::Edit;
		std::filesystem::path m_ScenePath = std::filesystem::path();

		EditorContext m_SelectedContext = {};

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
		float m_MenuBarHeight = 10.0f;
		ImVec2 m_LastMousePosition = ImVec2(0.0f, 0.0f);

		bool m_WindowDragging = false;
		bool m_TopResizing = false;
		bool m_LeftResizing = false;
		bool m_BottomResizing = false;
		bool m_RightResizing = false;
		bool m_Resizing = false;

		static EditorLayer* s_Instance;
	};
}
