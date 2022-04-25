#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class PropertiesPanel : public BasePanel
	{
	public:
		PropertiesPanel()
			: BasePanel(ICON_MDI_INFORMATION " Properties")
		{
			m_ID = "###" + std::to_string(s_ID);
			m_ID = m_Name + m_ID + m_Name;
			s_ID++;
		}

		PropertiesPanel(const char* name)
			: BasePanel(name)
		{
			m_ID = "###" + std::to_string(s_ID);
			m_ID = ICON_MDI_INFORMATION + m_Name + m_ID + m_Name;
			s_ID++;
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

	private:
		Entity m_Context;
		bool m_Locked = false;
		std::string m_ID;

		static uint32_t s_ID;
	};
}
