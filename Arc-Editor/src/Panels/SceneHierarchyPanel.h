#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>

#include "BasePanel.h"

struct ImRect;

namespace ArcEngine
{
	class SceneHierarchyPanel : public BasePanel
	{
	public:
		explicit SceneHierarchyPanel(const char* name = "Hierarchy");
		~SceneHierarchyPanel() override = default;

		SceneHierarchyPanel(const SceneHierarchyPanel& other) = delete;
		SceneHierarchyPanel(SceneHierarchyPanel&& other) = delete;
		SceneHierarchyPanel& operator=(const SceneHierarchyPanel& other) = delete;
		SceneHierarchyPanel& operator=(SceneHierarchyPanel&& other) = delete;

		void OnUpdate([[maybe_unused]] Timestep ts, WorkQueue* queue) override;
		void OnImGuiRender(WorkQueue* queue) override;

		void SetContext(Scene* context);

	private:
		ImRect DrawEntityNode(UUID id, TagComponent& tc, const RelationshipComponent& rc, Entity entity, uint32_t depth = 0, bool forceExpandTree = false, bool isPartOfPrefab = false);
		void DrawContextMenu() const;
		void DragDropTarget() const;

		friend class SceneViewport;

	private:
		Scene* m_Context = nullptr;
		Entity m_SelectedEntity;
		Entity m_RenamingEntity;
		Entity m_DeletedEntity;
		Entity m_DraggedEntity;
		Entity m_DraggedEntityTarget;

		ImGuiTextFilter m_Filter;
	};
}
