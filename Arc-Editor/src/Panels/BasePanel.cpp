#include "BasePanel.h"

#include <imgui/imgui_internal.h>

#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	uint32_t BasePanel::s_Count = 0;

	BasePanel::BasePanel(const char* name, const char* icon, bool defaultShow)
		: Showing(defaultShow), m_Name(name), m_Icon(icon)
	{
		ARC_PROFILE_SCOPE();

		m_ID = fmt::format(" {} {}\t\t###{}{}", icon, name, s_Count, name).c_str();
		s_Count++;
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
