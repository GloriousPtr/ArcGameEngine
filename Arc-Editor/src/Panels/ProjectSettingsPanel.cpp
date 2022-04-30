#include "ProjectSettingsPanel.h"

#include <ArcEngine.h>
#include <imgui/imgui.h>

namespace ArcEngine
{
	void ProjectSettingsPanel::OnImGuiRender()
	{
		ImGui::Begin(m_ID.c_str(), &m_Showing);

		static void* instance = nullptr;
		if (ImGui::Button("Make Instance!"))
		{
			instance = ScriptingEngine::MakeInstance("Test");
		}

		if (ImGui::Button("Test Call 1"))
		{
			ScriptingEngine::Call(instance, "Test", "DoSomething()", nullptr);
		}

		static int times = 0;
		if (ImGui::Button("Test Call 2"))
		{
			times++;

			void* args[1];
			args[0] = &times;
			ScriptingEngine::Call(instance, "Test", "DoSomething(int)", args);
		}

		ImGui::End();
	}
}
