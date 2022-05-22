#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class StatsPanel : public BasePanel
	{
	public:
		StatsPanel(const char* name = "Stats")
			: BasePanel(name, ICON_MDI_INFORMATION_VARIANT, true)
		{}

		virtual void OnImGuiRender() override;

	private:
		float m_Time = 0.0f;
		float m_FpsValues[50];
		eastl::vector<float> m_FrameTimes;
	};
}
