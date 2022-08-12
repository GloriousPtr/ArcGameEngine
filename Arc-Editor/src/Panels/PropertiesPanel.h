#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"
#include "EditorContext.h"
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

		void SetContext(const EditorContext context)
		{
			if (m_Locked && m_Context.Data)
				return;

			m_Context = context;
		}

		void ForceSetContext(const EditorContext context) { m_Context = context; }

	private:
		void DrawComponents(Entity entity);
		void DrawFileProperties(const char* filepath);

		template<typename Component>
		void DrawAddComponent(Entity entity, const char* name);

	private:
		EditorContext m_Context = {};
		bool m_Locked = false;
	};
}
