#pragma once

#include "BasePanel.h"

namespace ArcEngine
{
	class RendererSettingsPanel : public BasePanel
	{
	public:
		explicit RendererSettingsPanel(const char* name = "Render Settings");
		virtual ~RendererSettingsPanel() override = default;

		RendererSettingsPanel(const RendererSettingsPanel& other) = delete;
		RendererSettingsPanel(RendererSettingsPanel&& other) = delete;
		RendererSettingsPanel& operator=(const RendererSettingsPanel& other) = delete;
		RendererSettingsPanel& operator=(RendererSettingsPanel&& other) = delete;

		virtual void OnImGuiRender() override;
	};
}
