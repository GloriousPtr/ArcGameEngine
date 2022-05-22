#include "BasePanel.h"

namespace ArcEngine
{
	static uint32_t s_ID = 0;

	BasePanel::BasePanel(const char* name, const char* icon, bool defaultShow)
		: m_Name(name), m_Icon(icon), m_Showing(defaultShow)
	{
		ARC_PROFILE_SCOPE();

		std::string tmp = "###" + std::to_string(s_ID);
		m_ID = icon + eastl::string(" ") + m_Name + tmp.c_str() + m_Name;
		s_ID++;
	}
}
