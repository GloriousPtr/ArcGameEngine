#pragma once

#include <ArcEngine.h>
#include <imgui/imgui.h>

#include "BasePanel.h"

namespace ArcEngine
{
	class ConsolePanel : public BasePanel
	{
	public:
		struct Message
		{
			eastl::string ID;
			eastl::string Buffer;
			Log::Level Level;

			Message(const eastl::string& id, const eastl::string& message = "", Log::Level level = Log::Level::Trace);
			void OnImGuiRender() const;

			static const char* GetLevelName(Log::Level level);
			static glm::vec4 GetRenderColor(Log::Level level);
			static const char* GetLevelIcon(Log::Level level);
		};

	public:
		explicit ConsolePanel(const char* name = "Console");
		virtual ~ConsolePanel() override;

		ConsolePanel(const ConsolePanel& other) = delete;
		ConsolePanel(ConsolePanel&& other) = delete;
		ConsolePanel& operator=(const ConsolePanel& other) = delete;
		ConsolePanel& operator=(ConsolePanel&& other) = delete;

		void AddMessage(const eastl::string& message, Log::Level level);
		const Message* GetRecentMessage();
		void Clear();
		void SetFocus() const;

		virtual void OnImGuiRender() override;

	private:
		void ImGuiRenderHeader();
		void ImGuiRenderSettings();
		void ImGuiRenderMessages();

	private:
		float m_DisplayScale = 1.0f;
		uint16_t m_Capacity = 200;
		uint16_t m_BufferSize = 0;
		uint16_t m_BufferBegin = 0;
		uint32_t s_MessageBufferRenderFilter = (uint32_t)Log::Level::Trace;
		bool m_AllowScrollingToBottom = true;
		bool m_RequestScrollToBottom = false;
		eastl::vector<Ref<Message>> m_MessageBuffer;
		ImGuiTextFilter m_Filter;
	};
}
