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
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(btnTitle).x - ImGui::GetStyle().WindowPadding.x);

			if (ImGui::Button(btnTitle))
				ScriptEngine::ReloadAppDomain();

			OnEnd();
		}
	}
}
