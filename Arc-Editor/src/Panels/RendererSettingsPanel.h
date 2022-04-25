#pragma once
#include <ArcEngine.h>

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class RendererSettingsPanel : public BasePanel
	{
	public:
		RendererSettingsPanel()
			: BasePanel(ICON_MDI_GPU " Render Settings")
		{}

		virtual ~RendererSettingsPanel() override = default;

		virtual void OnImGuiRender() override;
	};
}
