#pragma once

#include "BasePanel.h"
#include "EditorContext.h"

#include <imgui/imgui.h>

namespace ArcEngine
{
	class PropertiesPanel : public BasePanel
	{
	public:
		explicit PropertiesPanel(const char* name = "Properties");

		~PropertiesPanel() override = default;

		PropertiesPanel(const PropertiesPanel& other) = delete;
		PropertiesPanel(PropertiesPanel&& other) = delete;
		PropertiesPanel& operator=(const PropertiesPanel& other) = delete;
		PropertiesPanel& operator=(PropertiesPanel&& other) = delete;

		void OnImGuiRender() override;

		void SetContext(const EditorContext& context)
		{
			if ((m_Locked && m_Context) || context == m_Context)
				return;

			m_Context = context;
		}

		void ForceSetContext(const EditorContext& context) { m_Context = context; }

	private:
		void DrawComponents(Entity entity);
		void DrawFileProperties(const char* filepath);

		template<typename Component>
		void DrawAddComponent(Entity entity, const char8_t* name, const char* category = nullptr) const;

	private:
		EditorContext m_Context = {};
		bool m_Locked = false;
		ImGuiTextFilter m_Filter;
		Ref<Scene> m_Scene;
	};
}
