#include "ProjectSettingsPanel.h"

#include <ArcEngine.h>
#include <imgui/imgui.h>

#include "../Utils/UI.h"

namespace ArcEngine
{
	void ProjectSettingsPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		if (OnBegin())
		{
			static const char* btnTitle = "Reload Assemblies";
			static const float padding = ImGui::GetStyle().WindowPadding.x;
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(btnTitle).x - padding);

			if (ImGui::Button(btnTitle))
				ScriptEngine::ReloadAppDomain();

			OnEnd();
		}
	}
}
