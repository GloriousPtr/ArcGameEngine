#pragma once

#include "BasePanel.h"

namespace ArcEngine
{
	class ProjectSettingsPanel : public BasePanel
	{
	public:
		ProjectSettingsPanel();

		~ProjectSettingsPanel() override = default;
		
		ProjectSettingsPanel(const ProjectSettingsPanel& other) = delete;
		ProjectSettingsPanel(ProjectSettingsPanel&& other) = delete;
		ProjectSettingsPanel& operator=(const ProjectSettingsPanel& other) = delete;
		ProjectSettingsPanel& operator=(ProjectSettingsPanel&& other) = delete;

		void OnImGuiRender(WorkQueue* queue) override;
	};
}
