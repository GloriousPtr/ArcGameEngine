#include "ConsolePanel.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <glm/glm.hpp>

#include "Arc/ImGui/Modules/ExternalConsoleSink.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	ConsolePanel::ConsolePanel()
		: BasePanel(ICON_MDI_CONSOLE " Console")
	{
        s_MessageBufferRenderFilter |= LogLevel::Trace;
		s_MessageBufferRenderFilter |= LogLevel::Info;
		s_MessageBufferRenderFilter |= LogLevel::Debug;
		s_MessageBufferRenderFilter |= LogLevel::Warn;
		s_MessageBufferRenderFilter |= LogLevel::Error;
		s_MessageBufferRenderFilter |= LogLevel::Critical;

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
			(*message) = nullptr;

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
		ImGui::SetWindowFocus(m_Name.c_str());
	}

	void ConsolePanel::ImGuiRenderHeader()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		if (ImGui::Button(ICON_MDI_COGS))
			ImGui::OpenPopup("SettingsPopup");
		if (ImGui::BeginPopup("SettingsPopup"))
		{
			ImGuiRenderSettings();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_MAGNIFY);
		ImGui::SameLine();

		float spacing = ImGui::GetStyle().ItemSpacing.x;
        ImGui::GetStyle().ItemSpacing.x = 2;
        float levelButtonWidth = (ImGui::CalcTextSize(Message::GetLevelIcon(LogLevel(1))) + ImGui::GetStyle().FramePadding * 2.0f).x;
        float levelButtonWidths = (levelButtonWidth + ImGui::GetStyle().ItemSpacing.x) * 7;

		{
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[1];
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
			m_Filter.Draw("###ConsoleFilter", ImGui::GetContentRegionAvail().x - (levelButtonWidths));
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[0];
		}

		for(int i = 0; i < 6; i++)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::SameLine();
			auto level = LogLevel(glm::pow(2, i));

			bool levelEnabled = s_MessageBufferRenderFilter & level;
			glm::vec4 c = Message::GetRenderColor(level);
			if(levelEnabled)
				ImGui::PushStyleColor(ImGuiCol_Text, { c.r, c.g, c.b, c.a });
			else
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5, 0.5f, 0.5f));

			if(ImGui::Button(Message::GetLevelIcon(level)))
			{
				s_MessageBufferRenderFilter ^= level;
			}

			if(ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", Message::GetLevelName(level));
			}
			ImGui::PopStyleColor(2);
		}

		ImGui::SameLine();

		if (ImGui::Button(ICON_MDI_NOTIFICATION_CLEAR_ALL))
			Clear();

		ImGui::GetStyle().ItemSpacing.x = spacing;

		if(!m_Filter.IsActive())
		{
			ImGui::SameLine();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[1];
			ImGui::SetCursorPosX(ImGui::GetFontSize() * 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));
			ImGui::TextUnformatted("Search...");
			ImGui::PopStyleVar();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[0];
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
					if (!(s_MessageBufferRenderFilter & (*message)->Level))
						continue;

					if (m_Filter.IsActive())
					{
						auto& m = (*message);
						if (m_Filter.PassFilter(m->Buffer.c_str()))
						{
							m->OnImGuiRender();
						}
					}
					else
					{
						(*message)->OnImGuiRender();
					}
				}
			}

			if (m_BufferBegin != 0) // Skipped first messages in vector
			{
				for (auto message = m_MessageBuffer.begin(); message != messageStart; message++)
				{
					if (!(s_MessageBufferRenderFilter & (*message)->Level))
						continue;

					if(m_Filter.IsActive())
					{
						if(m_Filter.PassFilter((*message)->Buffer.c_str()))
						{
							(*message)->OnImGuiRender();
						}
					}
					else
					{
						(*message)->OnImGuiRender();
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

	ConsolePanel::Message::Message(const std::string message, LogLevel level)
		: Buffer(message), Level(level)
	{
	}

	void ConsolePanel::Message::OnImGuiRender()
	{
		glm::vec4 c = GetRenderColor(Level);
		ImGui::PushStyleColor(ImGuiCol_Text, { c.r, c.g, c.b, c.a });
		auto levelIcon = GetLevelIcon(Level);
		ImGui::TextUnformatted(levelIcon);
		ImGui::SameLine();
		ImGui::TextUnformatted(Buffer.c_str());
		ImGui::PopStyleColor();

		if(ImGui::BeginPopupContextItem(Buffer.c_str()))
		{
			if(ImGui::MenuItem("Copy"))
				ImGui::SetClipboardText(Buffer.c_str());

			ImGui::EndPopup();
		}

		if(ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", Buffer.c_str());
		}
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
			case LogLevel::Error:			return { 0.90f, 0.25f, 0.25f, 1.00f }; // Red
			case LogLevel::Critical:		return { 0.60f, 0.20f, 0.80f, 1.00f }; // Purple
		}
		return { 1.00f, 1.00f, 1.00f, 1.00f };
	}

	const char* ConsolePanel::Message::GetLevelIcon(LogLevel level)
    {
        switch(level)
        {
        case LogLevel::Trace:				return ICON_MDI_MESSAGE_TEXT;
        case LogLevel::Info:				return ICON_MDI_INFORMATION;
        case LogLevel::Debug:				return ICON_MDI_BUG;
        case LogLevel::Warn:				return ICON_MDI_ALERT;
        case LogLevel::Error:				return ICON_MDI_CLOSE_OCTAGON;
        case LogLevel::Critical:			return ICON_MDI_ALERT_OCTAGRAM;
        default:
            return "Unknown name";
        }
    }
}
