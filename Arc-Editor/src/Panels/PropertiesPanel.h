#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class PropertiesPanel : public BasePanel
	{
	public:
		PropertiesPanel(const char* name = "Properties")
			: BasePanel(name, ICON_MDI_INFORMATION, true)
		{
		}

		virtual ~PropertiesPanel() override = default;

		virtual void OnImGuiRender() override;

		void SetContext(const Entity entity)
		{
			if (m_Locked && m_Context)
				return;

			m_Context = entity;
		}

		void ForceSetContext(const Entity entity) { m_Context = entity; }

	private:
		void DrawComponents(Entity entity);

		template<typename Component>
		void DrawAddComponent(Entity entity, const char* name);

	private:
		Entity m_Context;
		bool m_Locked = false;
	};
}
