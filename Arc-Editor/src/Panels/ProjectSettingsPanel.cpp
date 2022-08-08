#include "ProjectSettingsPanel.h"

#include <ArcEngine.h>
#include <imgui/imgui.h>

#include "../Utils/UI.h"

namespace ArcEngine
{
	eastl::string ProjectSettingsPanel::s_ScriptCoreAssemblyPath = "../Sandbox/Assemblies/Arc-ScriptCore.dll";
	eastl::string ProjectSettingsPanel::s_ScriptClientAssemblyPath = "../Sandbox/Assemblies/Sandbox.dll";

	void ProjectSettingsPanel::OnImGuiRender()
	{
		if (OnBegin())
		{
			UI::BeginProperties();
			UI::Property("Core Assembly Path", s_ScriptCoreAssemblyPath);
			UI::Property("Project Assembly Path", s_ScriptClientAssemblyPath);
			UI::EndProperties();

			static constexpr char* btnTitle = "Reload Assemblies";
			static const int padding = ImGui::GetStyle().WindowPadding.x;
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(btnTitle).x - padding);

			if (ImGui::Button(btnTitle))
				LoadAssemblies();

			OnEnd();
		}
	}

	void ProjectSettingsPanel::LoadAssemblies()
	{
		ScriptEngine::LoadCoreAssembly(s_ScriptCoreAssemblyPath.c_str());
		ScriptEngine::LoadClientAssembly(s_ScriptClientAssemblyPath.c_str());
	}
}
