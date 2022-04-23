#pragma once
#include <ArcEngine.h>

struct ImRect;

namespace ArcEngine
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();

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
