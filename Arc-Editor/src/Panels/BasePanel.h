#pragma once

#include <ArcEngine.h>

namespace ArcEngine
{
	class BasePanel
	{
	public:
		BasePanel(const char* name = "Unnamed Panel")
			: m_Name(name)
		{}

		virtual ~BasePanel() = default;

		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() = 0;
		
		bool IsShowing() { return m_Showing; }
		void SetShowing(bool value) { m_Showing = value; }
		const char* GetName() { return m_Name.c_str(); }

	protected:
		bool m_Showing = true;
		std::string m_Name;
	};
}
