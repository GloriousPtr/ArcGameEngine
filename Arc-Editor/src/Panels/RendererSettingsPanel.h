#pragma once
#include <ArcEngine.h>

#include "BasePanel.h"

namespace ArcEngine
{
	class RendererSettingsPanel : public BasePanel
	{
	public:
		RendererSettingsPanel()
			: BasePanel("Render Settings Panel")
		{}

		RendererSettingsPanel(const char* name)
			: BasePanel(name)
		{}

		virtual ~RendererSettingsPanel() override = default;

		virtual void OnImGuiRender() override;
	};
}
