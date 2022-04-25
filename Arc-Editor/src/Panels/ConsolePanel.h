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
			std::string Buffer;
			Log::Level Level;

			Message(const std::string message = "", Log::Level level = Log::Level::Trace);
			void OnImGuiRender();

			static const char* GetLevelName(Log::Level level);
			static glm::vec4 GetRenderColor(Log::Level level);
			static const char* GetLevelIcon(Log::Level level);
		};

	public:
		ConsolePanel(const char* name = "Console");
		virtual ~ConsolePanel() override = default;

		void AddMessage(std::string message, Log::Level level);
		const Message* GetRecentMessage();
		void Clear();
		void SetFocus();

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
		uint32_t s_MessageBufferRenderFilter = Log::Level::Trace;
		bool m_AllowScrollingToBottom = true;
		bool m_RequestScrollToBottom = false;
		std::vector<Ref<Message>> m_MessageBuffer;
		ImGuiTextFilter m_Filter;
	};
}
