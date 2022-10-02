#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"

namespace ArcEngine
{
	class StatsPanel : public BasePanel
	{
	public:
		explicit StatsPanel(const char* name = "Stats");
		~StatsPanel() = default;

		StatsPanel(const StatsPanel& other) = delete;
		StatsPanel(StatsPanel&& other) = delete;
		StatsPanel& operator=(const StatsPanel& other) = delete;
		StatsPanel& operator=(StatsPanel&& other) = delete;

		virtual void OnImGuiRender() override;

	private:
		float m_FpsValues[50];
		eastl::vector<float> m_FrameTimes;
	};
}
