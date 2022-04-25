#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class StatsPanel : public BasePanel
	{
	public:
		StatsPanel()
			: BasePanel(ICON_MDI_INFORMATION_VARIANT " Stats")
		{}

		virtual void OnImGuiRender() override;

	private:
		float m_Time = 0.0f;
		float m_FpsValues[50];
		std::vector<float> m_FrameTimes;
	};
}
