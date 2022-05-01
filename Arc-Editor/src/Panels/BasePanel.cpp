#include "BasePanel.h"

namespace ArcEngine
{
	static uint32_t s_ID = 0;

	BasePanel::BasePanel(const char* name, const char* icon, bool defaultShow)
		: m_Name(name), m_Icon(icon), m_Showing(defaultShow)
	{
		OPTICK_EVENT();

		m_ID = "###" + std::to_string(s_ID);
		m_ID = icon + std::string(" ") + m_Name + m_ID + m_Name;
		s_ID++;
	}
}
