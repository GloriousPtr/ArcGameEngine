#pragma once

#include <ArcEngine.h>

#include "BasePanel.h"

namespace ArcEngine
{
	class ConsolePanel : public BasePanel
	{
	public:
		struct Message
		{
			std::string Buffer;
			LogLevel Level;

			Message(const std::string message = "", LogLevel level = LogLevel::Trace);
			void OnImGuiRender();

			static LogLevel GetLowerLevel(LogLevel level);
			static LogLevel GetHigherLevel(LogLevel level);
			static const char* GetLevelName(LogLevel level);
			static glm::vec4 GetRenderColor(LogLevel level);
			static std::vector<LogLevel> Levels;
		};

	public:
		ConsolePanel();
		virtual ~ConsolePanel() override = default;

		void AddMessage(std::string message, LogLevel level);
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
		LogLevel m_Filter = LogLevel::Trace;
		bool m_AllowScrollingToBottom = true;
		bool m_RequestScrollToBottom = false;
		std::vector<Ref<Message>> m_MessageBuffer;
	};
}
