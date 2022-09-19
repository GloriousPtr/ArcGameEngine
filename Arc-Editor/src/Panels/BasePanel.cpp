#include "BasePanel.h"

#include <imgui/imgui_internal.h>

#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	uint32_t BasePanel::s_ID = 0;

	BasePanel::BasePanel(const char* name, const char* icon, bool defaultShow)
		: Showing(defaultShow), m_Name(name), m_Icon(icon)
	{
		ARC_PROFILE_SCOPE();

		static eastl::string space = " ";
		static eastl::string tab = "\t\t";

		std::string tmp = "###" + std::to_string(s_ID);
		m_ID = space + icon + space + m_Name + tab + tmp.c_str() + m_Name;
		s_ID++;
	}

	bool BasePanel::OnBegin(ImGuiWindowFlags flags)
	{
		if (!Showing)
			return false;

		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);

		ImGui::Begin(m_ID.c_str(), &Showing, flags | ImGuiWindowFlags_NoCollapse);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, EditorTheme::UIFramePadding);
		return true;
	}

	void BasePanel::OnEnd() const
	{
		ImGui::PopStyleVar();
		ImGui::End();
	}
}
