#pragma once

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class ProjectSettingsPanel : public BasePanel
	{
	public:
		ProjectSettingsPanel()
			: BasePanel("Project Settings", ICON_MDI_SETTINGS)
		{
		}

		virtual ~ProjectSettingsPanel() override = default;

		virtual void OnImGuiRender() override;

		static void LoadAssemblies();

	private:
		static std::string s_ScriptCoreAssemblyPath;
		static std::string s_ScriptClientAssemblyPath;
	};
}
