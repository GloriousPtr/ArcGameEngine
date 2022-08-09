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

		Entity GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(const Entity entity) { m_SelectionContext = entity; }

	private:
		ImRect DrawEntityNode(Entity entity, bool skipChildren = false);

	private:
		Ref<Scene> m_Context = nullptr;
		Entity m_SelectionContext;
		Entity m_DeletedEntity;
		uint32_t m_CurrentlyVisibleEntities;
	};
}
