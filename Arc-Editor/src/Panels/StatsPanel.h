#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"

namespace ArcEngine
{
	class StatsPanel : public BasePanel
	{
	public:
		StatsPanel()
			: BasePanel("Stats Panel")
		{}

		StatsPanel(const char* name)
			: BasePanel(name)
		{}

		virtual void OnImGuiRender() override;

	private:
		float m_Time = 0.0f;
		float m_FpsValues[50];
		std::vector<float> m_FrameTimes;
	};
}
