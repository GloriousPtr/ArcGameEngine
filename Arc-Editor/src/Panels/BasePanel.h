#pragma once

#include <ArcEngine.h>

namespace ArcEngine
{
	class BasePanel
	{
	public:
		BasePanel(const char* name = "Unnamed Panel", const char* icon = "");

		virtual ~BasePanel() = default;

		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() = 0;
		
		bool IsShowing() { return m_Showing; }
		void SetShowing(bool value) { m_Showing = value; }
		const char* GetName() { return m_Name.c_str(); }
		const char* GetIcon() { return m_Icon; }

	protected:
		bool m_Showing = true;
		std::string m_Name;
		std::string m_ID;
		const char* m_Icon;
	};
}
