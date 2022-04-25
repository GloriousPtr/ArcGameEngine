#include "BasePanel.h"

namespace ArcEngine
{
	static uint32_t s_ID = 0;

	BasePanel::BasePanel(const char* name, const char* icon)
		: m_Name(name)
	{
		OPTICK_EVENT();

		m_ID = "###" + std::to_string(s_ID);
		m_ID = icon + std::string(" ") + m_Name + m_ID + m_Name;
		s_ID++;

		m_Icon = icon;
	}
}
