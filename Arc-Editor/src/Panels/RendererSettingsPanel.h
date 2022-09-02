#pragma once

#include <ArcEngine.h>
#include <icons/IconsMaterialDesignIcons.h>

#include "BasePanel.h"

namespace ArcEngine
{
	class RendererSettingsPanel : public BasePanel
	{
	public:
		explicit RendererSettingsPanel(const char* name = "Render Settings")
			: BasePanel(name, ICON_MDI_GPU)
		{}

		virtual ~RendererSettingsPanel() override = default;

		RendererSettingsPanel(const RendererSettingsPanel& other) = delete;
		RendererSettingsPanel(RendererSettingsPanel&& other) = delete;
		RendererSettingsPanel& operator=(const RendererSettingsPanel& other) = delete;
		RendererSettingsPanel& operator=(RendererSettingsPanel&& other) = delete;

		virtual void OnImGuiRender() override;
	};
}
