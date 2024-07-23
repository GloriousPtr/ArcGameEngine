#pragma once

#include <ArcEngine.h>

#include "Panels/AssetPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/EditorContext.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/SceneViewport.h"

namespace ArcEngine
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate([[maybe_unused]] Timestep ts, WorkQueue* queue) override;
		void OnImGuiRender(WorkQueue* queue) override;
		void OnEvent([[maybe_unused]] Event& e) override;

		void SetContext(EditorContextType type, const char* data, size_t size) { m_SelectedContext.Set(type, data, size); }
		void ResetContext() { m_SelectedContext.Reset(); }
		[[nodiscard]] const EditorContext& GetContext() const { return m_SelectedContext; }
		
		void OpenScene(eastl::string_view filepath);

		static EditorLayer* GetInstance() { return s_Instance; }

	private:
		void BeginDockspace(const char* name) const;
		void EndDockspace() const;
		bool OnKeyPressed([[maybe_unused]] const KeyPressedEvent& e);
		[[nodiscard]] bool OnMouseButtonPressed([[maybe_unused]] const MouseButtonPressedEvent& e) const;
		[[nodiscard]] bool OnMouseButtonReleased([[maybe_unused]] const MouseButtonReleasedEvent& e) const;

		void OpenProject(eastl::string_view path);
		void OpenProject();
		void SaveProject(const std::filesystem::path& path) const;

		void NewScene();
		void SaveScene();
		void SaveSceneAs();

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void OnSceneUnpause();

	private:
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		enum class SceneState { Edit, Play, Pause, Step };
		SceneState m_SceneState = SceneState::Edit;
		std::filesystem::path m_ScenePath = std::filesystem::path();

		std::string m_TempProjectName;
		std::string m_TempProjectPath;

		EditorContext m_SelectedContext = {};

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ConsolePanel m_ConsolePanel;

		eastl::vector<Scope<BasePanel>> m_Panels;
		eastl::vector<Scope<SceneViewport>> m_Viewports;
		eastl::vector<Scope<PropertiesPanel>> m_Properties;
		eastl::vector<Scope<AssetPanel>> m_AssetPanels;

		glm::vec2 m_LastMousePosition = glm::vec2(0.0f);
		float m_TopMenuBarHeight = 6.0f;
		bool m_ShowNewProjectModal = false;
		bool m_ShowSceneHierarchyPanel = true;
		bool m_Fullscreen = false;
		bool m_ShowDemoWindow = false;

		static EditorLayer* s_Instance;
	};
}
