#pragma once

#include <ArcEngine.h>

#include <imgui/imgui.h>

namespace ArcEngine
{
	class BasePanel
	{
	public:
		bool Showing;

	public:
		BasePanel(const char* name = "Unnamed Panel", const char* icon = "", bool defaultShow = false);

		virtual ~BasePanel() = default;

		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() = 0;
		
		const char* GetName() { return m_Name.c_str(); }
		const char* GetIcon() { return m_Icon; }

	protected:
		bool OnBegin(ImGuiWindowFlags flags = 0);
		void OnEnd();

	protected:
		eastl::string m_Name;
		const char* m_Icon;
		eastl::string m_ID;
	};
}
