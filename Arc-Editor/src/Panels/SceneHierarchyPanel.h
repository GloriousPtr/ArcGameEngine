#pragma once
#include <ArcEngine.h>

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

struct ImRect;

namespace ArcEngine
{
	class SceneHierarchyPanel : public BasePanel
	{
	public:
		SceneHierarchyPanel(const char* name = "Hierarchy")
			: BasePanel(name, ICON_MDI_VIEW_LIST, true)
		{
		}

		virtual ~SceneHierarchyPanel() = default;

		void SetContext(const Ref<Scene>& context);

		virtual void OnImGuiRender() override;

	private:
		ImRect DrawEntityNode(Entity entity, bool skipChildren = false, uint32_t depth = 0, bool forceExpandTree = false, bool isPartOfPrefab = false);
		void SceneHierarchyPanel::DrawContextMenu();

	private:
		Ref<Scene> m_Context = nullptr;
		Entity m_DeletedEntity;
		Entity m_DraggedEntity;
		Entity m_DraggedEntityTarget;

		ImGuiTextFilter m_Filter;
	};
}
