#include "ConsolePanel.h"

#include <Arc/ImGui/Modules/ExternalConsoleSink.h>

#ifdef ARC_PLATFORM_VISUAL_STUDIO
#include <Platform/VisualStudio/VisualStudioAccessor.h>
#endif //ARC_PLATFORM_VISUAL_STUDIO

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <icons/IconsMaterialDesignIcons.h>

#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	ConsolePanel::ConsolePanel(const char* name)
		: BasePanel(name, ICON_MDI_CONSOLE, true)
	{
		ARC_PROFILE_SCOPE();

		s_MessageBufferRenderFilter |= Log::Level::Trace;
		s_MessageBufferRenderFilter |= Log::Level::Info;
		s_MessageBufferRenderFilter |= Log::Level::Debug;
		s_MessageBufferRenderFilter |= Log::Level::Warn;
		s_MessageBufferRenderFilter |= Log::Level::Error;
		s_MessageBufferRenderFilter |= Log::Level::Critical;

		ExternalConsoleSink::SetConsoleSink_HandleFlush([this](std::string_view message, const char* filepath, const char* function, int32_t line, Log::Level level){ AddMessage(message, filepath, function, line, level); });
		m_MessageBuffer = std::vector<Ref<ConsolePanel::Message>>(m_Capacity);
	}

	ConsolePanel::~ConsolePanel()
	{
		ExternalConsoleSink::SetConsoleSink_HandleFlush(nullptr);
	}

	void ConsolePanel::AddMessage(const std::string_view message, const char* filepath, const char* function, int32_t line, Log::Level level)
	{
		ARC_PROFILE_SCOPE();

		static uint32_t id = 0;

		*(m_MessageBuffer.begin() + m_BufferBegin) = CreateRef<Message>(id, message, filepath, function, line, level);
		if (++m_BufferBegin == m_Capacity)
			m_BufferBegin = 0;
		if (m_BufferSize < m_Capacity)
			m_BufferSize++;

		// Request to scroll to bottom of the list to view the new message
		m_RequestScrollToBottom = m_AllowScrollingToBottom;

		id++;
	}

	const ConsolePanel::Message* ConsolePanel::GetRecentMessage() const
	{
		ARC_PROFILE_SCOPE();

		if (m_BufferBegin == 0)
			return nullptr;
		
		return (m_MessageBuffer.begin() + m_BufferBegin - 1)->get();
	}

	void ConsolePanel::Clear()
	{
		ARC_PROFILE_SCOPE();

		for (auto& message : m_MessageBuffer)
			message = nullptr;

		m_BufferBegin = 0;
	}

	void ConsolePanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		if (OnBegin())
		{
			ImGuiRenderHeader();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGuiRenderMessages();

			OnEnd();
		}
	}

	void ConsolePanel::SetFocus() const
	{
		ARC_PROFILE_SCOPE();

		ImGui::SetWindowFocus(m_ID.c_str());
	}

	void ConsolePanel::ImGuiRenderHeader()
	{
		ARC_PROFILE_SCOPE();

		if (ImGui::Button(StringUtils::FromChar8T(ICON_MDI_COGS)))
			ImGui::OpenPopup("SettingsPopup");

		if (ImGui::BeginPopup("SettingsPopup"))
		{
			ImGuiRenderSettings();
			ImGui::EndPopup();
		}

		ImGui::SameLine();

		const float spacing = ImGui::GetStyle().ItemSpacing.x;
        ImGui::GetStyle().ItemSpacing.x = 2;
        const float levelButtonWidth = (ImGui::CalcTextSize(StringUtils::FromChar8T(Message::GetLevelIcon(Log::Level::Trace))) + ImGui::GetStyle().FramePadding * 2.0f).x;
        const float levelButtonWidths = (levelButtonWidth + ImGui::GetStyle().ItemSpacing.x) * 7;

		const float cursorPosX = ImGui::GetCursorPosX();
		m_Filter.Draw("###ConsoleFilter", ImGui::GetContentRegionAvail().x - levelButtonWidths);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		for (int i = 0; i < 6; i++)
		{
			ImGui::SameLine();
			const auto level = static_cast<Log::Level>(glm::pow(2, i));

			const bool levelEnabled = s_MessageBufferRenderFilter & level;
			glm::vec4 c = Message::GetRenderColor(level);
			if(levelEnabled)
				ImGui::PushStyleColor(ImGuiCol_Text, { c.r, c.g, c.b, c.a });
			else
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5, 0.5f, 0.5f));

			if(ImGui::Button(StringUtils::FromChar8T(Message::GetLevelIcon(level))))
			{
				s_MessageBufferRenderFilter ^= level;
			}

			if(ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", Message::GetLevelName(level));
			}
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();

		if (ImGui::Button(StringUtils::FromChar8T(ICON_MDI_NOTIFICATION_CLEAR_ALL)))
			Clear();

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		ImGui::GetStyle().ItemSpacing.x = spacing;

		if (!m_Filter.IsActive())
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(cursorPosX + ImGui::GetFontSize() * 0.5f);
			ImGui::TextUnformatted(StringUtils::FromChar8T(ICON_MDI_MAGNIFY " Search..."));
		}
	}

	void ConsolePanel::ImGuiRenderSettings()
	{
		ARC_PROFILE_SCOPE();

		UI::BeginProperties(ImGuiTableFlags_SizingStretchSame);
		UI::Property("Scroll to bottom", m_AllowScrollingToBottom);
		UI::Property("DisplayScale", m_DisplayScale, 0.5f, 4.0f, nullptr, 0.1f, "%.1f");
		UI::EndProperties();
	}

	void ConsolePanel::ImGuiRenderMessages()
	{
		ARC_PROFILE_SCOPE();

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg
			| ImGuiTableFlags_ContextMenuInBody
			| ImGuiTableFlags_ScrollY;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 1, 1 });
		if (ImGui::BeginTable("ScrollRegionTable", 1, tableFlags))
		{
			ImGui::SetWindowFontScale(m_DisplayScale);

			const auto& messageStart = m_MessageBuffer.begin() + m_BufferBegin;
			if (*messageStart) // If contains old message here
			{
				for (auto message = messageStart; message != m_MessageBuffer.end(); ++message)
				{
					if (!(s_MessageBufferRenderFilter & (*message)->Level))
						continue;

					if (m_Filter.IsActive())
					{
						const auto& m = *message;
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
				for (auto message = m_MessageBuffer.begin(); message != messageStart; ++message)
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

			ImGui::EndTable();
		}
		ImGui::PopStyleVar();
	}

	ConsolePanel::Message::Message(uint32_t id, std::string_view message, const char* filepath, const char* function, int32_t line, Log::Level level)
		: ID(id), Buffer(message.data(), message.size()), Filepath(filepath ? filepath : ""), Function(function ? function : ""), Line(line), Level(level)
	{
		ARC_PROFILE_SCOPE();
	}

	void ConsolePanel::Message::OnImGuiRender() const
	{
		ARC_PROFILE_SCOPE();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGuiTreeNodeFlags flags = 0;
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth;
		flags |= ImGuiTreeNodeFlags_FramePadding;
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		ImGui::PushID(static_cast<int>(ID));
		glm::vec4 c = GetRenderColor(Level);
		ImGui::PushStyleColor(ImGuiCol_Text, { c.r, c.g, c.b, c.a });
		ImGui::PushFont(EditorTheme::BoldFont);
		const auto levelIcon = GetLevelIcon(Level);
		ImGui::TreeNodeEx(&ID, flags, "%s  %s", StringUtils::FromChar8T(levelIcon), Buffer.c_str());
		ImGui::PopFont();
		ImGui::PopStyleColor();

#ifdef ARC_PLATFORM_VISUAL_STUDIO
		if (Line != 0 && !Filepath.empty() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			VisualStudioAccessor::OpenFile(Filepath, Line);
#endif //ARC_PLATFORM_VISUAL_STUDIO

		if(ImGui::BeginPopupContextItem("Popup"))
		{
			if(ImGui::MenuItem("Copy"))
				ImGui::SetClipboardText(Buffer.c_str());

			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

	const char* ConsolePanel::Message::GetLevelName(Log::Level level)
	{
		switch (level)
		{
			case Log::Level::Trace:			return "Trace";
			case Log::Level::Info:			return "Info";
			case Log::Level::Debug:			return "Debug";
			case Log::Level::Warn:			return "Warning";
			case Log::Level::Error:			return "Error";
			case Log::Level::Critical:		return "Critical";
		}

		return "Unknown name";
	}

	glm::vec4 ConsolePanel::Message::GetRenderColor(Log::Level level)
	{
		switch (level)
		{
		case Log::Level::Trace:
										{
											const auto& txtColor = EditorTheme::TextColor;
											return { txtColor.x, txtColor.y, txtColor.z, txtColor.w }; // Grey
										}
			case Log::Level::Info:			return { 0.10f, 0.60f, 0.10f, 1.00f }; // Green
			case Log::Level::Debug:			return { 0.00f, 0.50f, 0.50f, 1.00f }; // Cyan
			case Log::Level::Warn:			return { 0.60f, 0.60f, 0.10f, 1.00f }; // Yellow
			case Log::Level::Error:			return { 0.90f, 0.25f, 0.25f, 1.00f }; // Red
			case Log::Level::Critical:		return { 0.60f, 0.20f, 0.80f, 1.00f }; // Purple
		}

		return { 1.00f, 1.00f, 1.00f, 1.00f };
	}

	const char8_t* ConsolePanel::Message::GetLevelIcon(Log::Level level)
    {
        switch(level)
        {
			case Log::Level::Trace:				return ICON_MDI_MESSAGE_TEXT;
			case Log::Level::Info:				return ICON_MDI_INFORMATION;
			case Log::Level::Debug:				return ICON_MDI_BUG;
			case Log::Level::Warn:				return ICON_MDI_ALERT;
			case Log::Level::Error:				return ICON_MDI_CLOSE_OCTAGON;
			case Log::Level::Critical:			return ICON_MDI_ALERT_OCTAGRAM;
        }

		return u8"Unknown name";
    }
}
