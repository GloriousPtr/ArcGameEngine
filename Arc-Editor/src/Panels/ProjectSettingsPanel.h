#pragma once

#include <icons/IconsMaterialDesignIcons.h>

#include "BasePanel.h"

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
		
		ProjectSettingsPanel(const ProjectSettingsPanel& other) = delete;
		ProjectSettingsPanel(ProjectSettingsPanel&& other) = delete;
		ProjectSettingsPanel& operator=(const ProjectSettingsPanel& other) = delete;
		ProjectSettingsPanel& operator=(ProjectSettingsPanel&& other) = delete;

		virtual void OnImGuiRender() override;
	};
}
