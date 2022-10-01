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

		BasePanel(const BasePanel& other) = delete;
		BasePanel(BasePanel&& other) = delete;
		BasePanel& operator=(const BasePanel& other) = delete;
		BasePanel& operator=(BasePanel&& other) = delete;

		virtual void OnUpdate([[maybe_unused]] Timestep ts) { /* Not pure virtual */ }
		virtual void OnImGuiRender() = 0;
		
		const char* GetName() const { return m_Name.c_str(); }
		const char* GetIcon() const { return m_Icon; }

	protected:
		bool OnBegin(ImGuiWindowFlags flags = 0);
		void OnEnd() const;

	protected:
		eastl::string m_Name;
		const char* m_Icon;
		eastl::string m_ID;

	private:
		static uint32_t s_Count;
	};
}
