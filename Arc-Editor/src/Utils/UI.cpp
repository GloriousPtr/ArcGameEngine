#include "UI.h"

#include <ArcEngine.h>

#include <imgui/imgui.h>

#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui_internal.h"

namespace ArcEngine
{
	static int s_UIContextID = 0;
	static uint32_t s_Counter = 0;
	static char s_IDBuffer[16];

	void UI::PushID()
	{
		ImGui::PushID(s_UIContextID++);
		s_Counter = 0;
	}

	void UI::PopID()
	{
		ImGui::PopID();
		s_UIContextID--;
	}
	
	void UI::BeginPropertyGrid()
	{
		PushID();
		ImGui::Columns(2);
	}

	void UI::Property(const char* label)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		float fullWidth = ImGui::GetContentRegionAvail().x;
		float itemWidth = fullWidth * 0.6f;
		ImVec2 textSize = ImGui::CalcTextSize(label);
		ImRect textRect;
		textRect.Min = ImGui::GetCursorScreenPos();
		textRect.Max = textRect.Min;
		textRect.Max.x += fullWidth - itemWidth;
		textRect.Max.y += textSize.y;

		ImGui::SetCursorScreenPos(textRect.Min);

		ImGui::AlignTextToFramePadding();
		textRect.Min.y += window->DC.CurrLineTextBaseOffset;
		textRect.Max.y += window->DC.CurrLineTextBaseOffset;

		ImGui::ItemSize(textRect);
		if (ImGui::ItemAdd(textRect, window->GetID(label)))
		{
			ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), textRect.Min, textRect.Max, textRect.Max.x,
				textRect.Max.x, label, nullptr, &textSize);

			if (textRect.GetWidth() < textSize.x && ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", label);
		}
		ImVec2 v(0, textSize.y + window->DC.CurrLineTextBaseOffset);
		ImGui::SetCursorScreenPos(ImVec2(textRect.Max.x - v.x, textRect.Max.y - v.y));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(itemWidth);
	}

	bool UI::Property(const char* label, std::string& value)
	{
		bool modified = false;

		ImGui::PushID(label);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		char buffer[256];
		strcpy(buffer, value.c_str());

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::InputText(s_IDBuffer, buffer, 256))
		{
			value = buffer;
			modified = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::PopID();

		return modified;
	}

	void UI::Property(const char* label, const char* value)
	{
		ImGui::PushID(label);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		ImGui::InputText(s_IDBuffer, (char*)value, 256, ImGuiInputTextFlags_ReadOnly);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::PopID();
	}

	bool UI::Property(const char* label, int& value)
	{
		bool modified = false;

		ImGui::PushID(label);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragInt(s_IDBuffer, &value))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::PopID();

		return modified;
	}

	bool UI::Property(const char* label, int& value, int min, int max)
	{
		bool modified = false;

		ImGui::PushID(label);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderInt(s_IDBuffer, &value, min, max))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::PopID();

		return modified;
	}

	bool UI::Property(const char* label, float& value, float delta)
	{
		bool modified = false;

		ImGui::PushID(label);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat(s_IDBuffer, &value, delta))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::PopID();

		return modified;
	}

	bool UI::Property(const char* label, float& value, float min, float max, const char* fmt)
	{
		bool modified = false;

		ImGui::PushID(label);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderFloat(s_IDBuffer, &value, min, max, fmt))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::PopID();

		return modified;
	}

	bool UI::Property(const char* label, glm::vec2& value, float delta)
	{
		bool modified = false;

		ImGui::PushID(label);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat2(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::PopID();

		return modified;
	}

	bool UI::Property(const char* label, bool& flag)
	{
		bool modified = false;

		ImGui::PushID(label);
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::Checkbox(s_IDBuffer, &flag))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::PopID();

		return modified;
	}
	
	void UI::EndPropertyGrid()
	{
		ImGui::Columns(1);
		PopID();
	}

	void UI::DrawRowsBackground(int row_count, float line_height, float x1, float x2, float y_offset, uint32_t col_even, uint32_t col_odd)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		float y0 = ImGui::GetCursorScreenPos().y + (float)(int)y_offset;

		int row_display_start;
		int row_display_end;
		ImGui::CalcListClipping(row_count, line_height, &row_display_start, &row_display_end);
		for (int row_n = row_display_start; row_n < row_display_end; row_n++)
		{
			ImU32 col = (row_n & 1) ? col_odd : col_even;
			if ((col & IM_COL32_A_MASK) == 0)
				continue;
			float y1 = y0 + (line_height * row_n);
			float y2 = y1 + line_height;
			draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col);
		}
	}
}
