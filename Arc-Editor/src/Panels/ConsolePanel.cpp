#include "ConsolePanel.h"

#include <imgui/imgui.h>

#include "Arc/ImGui/Modules/ExternalConsoleSink.h"

namespace ArcEngine
{
	ConsolePanel::ConsolePanel()
		: BasePanel("Console")
	{
		ExternalConsoleSink<std::mutex>::SetConsoleSink_HandleFlush([&](std::string message, LogLevel level){ AddMessage(message, level); });
		m_MessageBuffer = std::vector<Ref<ConsolePanel::Message>>(m_Capacity);
	}

	void ConsolePanel::AddMessage(std::string message, LogLevel level)
	{
		*(m_MessageBuffer.begin() + m_BufferBegin) = CreateRef<Message>(message, level);
		if (++m_BufferBegin == m_Capacity)
			m_BufferBegin = 0;
		if (m_BufferSize < m_Capacity)
			m_BufferSize++;

		// Request to scroll to bottom of the list to view the new message
		m_RequestScrollToBottom = m_AllowScrollingToBottom;
	}

	const ConsolePanel::Message* ConsolePanel::GetRecentMessage()
	{
		if (m_BufferBegin == 0)
			return nullptr;
		
		return (m_MessageBuffer.begin() + m_BufferBegin - 1)->get();
	}

	void ConsolePanel::Clear()
	{
		for (auto message = m_MessageBuffer.begin(); message != m_MessageBuffer.end(); message++)
			(*message) = std::make_shared<Message>();
		m_BufferBegin = 0;
	}

	void ConsolePanel::OnImGuiRender()
	{
		ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
		ImGui::Begin(m_Name.c_str(), &m_Showing);
		{
			ImGuiRenderHeader();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGuiRenderMessages();
		}
		ImGui::End();
	}

	void ConsolePanel::SetFocus()
	{
		ImGui::SetWindowFocus("Console");
	}

	void ConsolePanel::ImGuiRenderHeader()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		const float spacing = style.ItemInnerSpacing.x;

		// Text change level
		//ImGui::AlignFirstTextHeightToWidgets();
		ImGui::Text("Display");

		ImGui::SameLine(0.0f, 2.0f * spacing);

		// Buttons to quickly change level left
		if (ImGui::ArrowButton("##MessageRenderFilter_L", ImGuiDir_Left))
		{
			m_Filter = Message::GetLowerLevel(m_Filter);
		}

		ImGui::SameLine(0.0f, spacing);

		// Dropdown with levels
		ImGui::PushItemWidth(ImGui::CalcTextSize("Critical").x * 1.36f);
		if (ImGui::BeginCombo(
			"##MessageRenderFilter",
			Message::GetLevelName(m_Filter),
			ImGuiComboFlags_NoArrowButton))
		{
			for (auto level = Message::Levels.begin(); level != Message::Levels.end(); level++)
			{
				bool is_selected = (m_Filter == *level);
				if (ImGui::Selectable(Message::GetLevelName(*level), is_selected))
					m_Filter = *level;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();

		ImGui::SameLine(0.0f, spacing);

		// Buttons to quickly change level right
		if (ImGui::ArrowButton("##MessageRenderFilter_R", ImGuiDir_Right))
		{
			m_Filter = Message::GetHigherLevel(m_Filter);
		}

		ImGui::SameLine(0.0f, spacing);

		// Button to clear the console
		ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x  - ImGui::CalcTextSize("Clear").x - ImGui::CalcTextSize("Settings").x - 2 * style.WindowPadding.x);
		if (ImGui::Button("Clear"))
			Clear();
		
		// Button for advanced settings
		ImGui::SameLine();
		if (ImGui::Button("Settings"))
			ImGui::OpenPopup("SettingsPopup");
		if (ImGui::BeginPopup("SettingsPopup"))
		{
			ImGuiRenderSettings();
			ImGui::EndPopup();
		}

	}

	void ConsolePanel::ImGuiRenderSettings()
	{
		const float maxWidth = ImGui::CalcTextSize("Scroll to bottom").x * 1.1f;
		const float spacing = ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(" ").x;

		// Checkbox for scrolling lock
		//ImGui::AlignFirstTextHeightToWidgets();
		ImGui::Text("Scroll to bottom");
		ImGui::SameLine(0.0f, spacing + maxWidth - ImGui::CalcTextSize("Scroll to bottom").x);
		ImGui::Checkbox("##ScrollToBottom", &m_AllowScrollingToBottom);

		// Slider for font scale
		//ImGui::AlignFirstTextHeightToWidgets();
		ImGui::Text("Display scale");
		ImGui::SameLine(0.0f, spacing + maxWidth - ImGui::CalcTextSize("Display scale").x);
		ImGui::PushItemWidth(maxWidth * 1.25f / 1.1f);
		ImGui::SliderFloat("##DisplayScale", &m_DisplayScale, 0.5f, 4.0f, "%.1f");
		ImGui::PopItemWidth();
	}

	void ConsolePanel::ImGuiRenderMessages()
	{
		ImGui::BeginChild("ScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		{
			ImGui::SetWindowFontScale(m_DisplayScale);

			auto messageStart = m_MessageBuffer.begin() + m_BufferBegin;
			if (*messageStart) // If contains old message here
			{
				for (auto message = messageStart; message != m_MessageBuffer.end(); message++)
				{
					auto& m = (*message);
					if (m->Level >= ConsolePanel::m_Filter)
					{
						m->OnImGuiRender();
					}
				}
			}

			if (m_BufferBegin != 0) // Skipped first messages in vector
			{
				for (auto message = m_MessageBuffer.begin(); message != messageStart; message++)
				{
					auto& m = (*message);
					if (m->Level >= ConsolePanel::m_Filter)
					{
						m->OnImGuiRender();
					}
				}
			}

			if (m_RequestScrollToBottom && ImGui::GetScrollMaxY() > 0)
			{
				ImGui::SetScrollY(ImGui::GetScrollMaxY());
				m_RequestScrollToBottom = false;
			}
		}
		ImGui::EndChild();
	}

	std::vector<LogLevel> ConsolePanel::Message::Levels
	{
		LogLevel::Trace,
		LogLevel::Debug,
		LogLevel::Info,
		LogLevel::Warn,
		LogLevel::Error,
		LogLevel::Critical,
		LogLevel::Off
	};

	ConsolePanel::Message::Message(const std::string message, LogLevel level)
		: Buffer(message), Level(level)
	{
	}

	void ConsolePanel::Message::OnImGuiRender()
	{
		glm::vec4 color = GetRenderColor(Level);
		ImGui::PushStyleColor(ImGuiCol_Text, { color.r, color.g, color.b, color.a });
		ImGui::TextUnformatted(Buffer.c_str());
		ImGui::PopStyleColor();
	}

	LogLevel ConsolePanel::Message::GetLowerLevel(LogLevel level)
	{
		switch (level)
		{
			case LogLevel::Off:				return LogLevel::Critical;
			case LogLevel::Critical:		return LogLevel::Error;
			case LogLevel::Error:			return LogLevel::Warn;
			case LogLevel::Warn:			//return LogLevel::Debug;
			case LogLevel::Debug:			return LogLevel::Info;
			case LogLevel::Info:
			case LogLevel::Trace:			return LogLevel::Trace;
		}
		return LogLevel::Trace;
	}

	LogLevel ConsolePanel::Message::GetHigherLevel(LogLevel level)
	{
		switch (level)
		{
			case LogLevel::Trace:			return LogLevel::Info;
			case LogLevel::Info:			//return LogLevel::Debug;
			case LogLevel::Debug:			return LogLevel::Warn;
			case LogLevel::Warn:			return LogLevel::Error;
			case LogLevel::Error:			return LogLevel::Critical;
			case LogLevel::Critical:
			case LogLevel::Off:				return LogLevel::Off;
		}
		return LogLevel::Trace;
	}

	const char* ConsolePanel::Message::GetLevelName(LogLevel level)
	{
		switch (level)
		{
			case LogLevel::Trace:			return "Trace";
			case LogLevel::Info:			return "Info";
			case LogLevel::Debug:			return "Debug";
			case LogLevel::Warn:			return "Warning";
			case LogLevel::Error:			return "Error";
			case LogLevel::Critical:		return "Critical";
			case LogLevel::Off:				return "None";
		}
		return "Unknown name";
	}

	glm::vec4 ConsolePanel::Message::GetRenderColor(LogLevel level)
	{
		switch (level)
		{
			case LogLevel::Trace:			return { 0.75f, 0.75f, 0.75f, 1.00f }; // White-ish gray
			case LogLevel::Info:			return { 0.20f, 0.80f, 0.20f, 1.00f }; // Green
			case LogLevel::Debug:			return { 0.00f, 0.50f, 0.50f, 1.00f }; // Cyan
			case LogLevel::Warn:			return { 0.80f, 0.80f, 0.20f, 1.00f }; // Yellow
			case LogLevel::Error:			return { 0.80f, 0.20f, 0.20f, 1.00f }; // Red
			case LogLevel::Critical:		return { 1.00f, 1.00f, 1.00f, 1.00f }; // White-white
		}
		return { 1.00f, 1.00f, 1.00f, 1.00f };
	}
}
