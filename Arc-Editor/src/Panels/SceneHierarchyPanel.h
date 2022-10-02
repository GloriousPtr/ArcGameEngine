#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"

struct ImRect;

namespace ArcEngine
{
	class SceneHierarchyPanel : public BasePanel
	{
	public:
		explicit SceneHierarchyPanel(const char* name = "Hierarchy");
		virtual ~SceneHierarchyPanel() = default;

		SceneHierarchyPanel(const SceneHierarchyPanel& other) = delete;
		SceneHierarchyPanel(SceneHierarchyPanel&& other) = delete;
		SceneHierarchyPanel& operator=(const SceneHierarchyPanel& other) = delete;
		SceneHierarchyPanel& operator=(SceneHierarchyPanel&& other) = delete;

		virtual void OnImGuiRender() override;

		void SetContext(const Ref<Scene>& context);

	private:
		ImRect DrawEntityNode(Entity entity, bool skipChildren = false, uint32_t depth = 0, bool forceExpandTree = false, bool isPartOfPrefab = false);
		void SceneHierarchyPanel::DrawContextMenu() const;
		void DragDropTarget() const;

		friend class SceneViewport;

	private:
		Ref<Scene> m_Context = nullptr;
		Entity m_DeletedEntity;
		Entity m_DraggedEntity;
		Entity m_DraggedEntityTarget;

		ImGuiTextFilter m_Filter;
	};
}
