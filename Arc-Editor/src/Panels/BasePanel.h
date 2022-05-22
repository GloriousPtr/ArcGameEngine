#pragma once

#include <ArcEngine.h>

namespace ArcEngine
{
	class BasePanel
	{
	public:
		BasePanel(const char* name = "Unnamed Panel", const char* icon = "", bool defaultShow = false);

		virtual ~BasePanel() = default;

		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() = 0;
		
		bool IsShowing() { return m_Showing; }
		void SetShowing(bool value) { m_Showing = value; }
		const char* GetName() { return m_Name.c_str(); }
		const char* GetIcon() { return m_Icon; }

	protected:
		eastl::string m_Name;
		const char* m_Icon;
		bool m_Showing;
		eastl::string m_ID;
	};
}
