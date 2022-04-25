#pragma once
#include <ArcEngine.h>

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class RendererSettingsPanel : public BasePanel
	{
	public:
		RendererSettingsPanel(const char* name = "Render Settings")
			: BasePanel(name, ICON_MDI_GPU)
		{}

		virtual ~RendererSettingsPanel() override = default;

		virtual void OnImGuiRender() override;
	};
}
