#include "UI.h"

#include <ArcEngine.h>

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui_internal.h>

#include "EditorTheme.h"

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
	
	void UI::BeginPropertyGrid(const char* label, const char* tooltip, bool rightAlignNextColumn)
	{
		PushID();
		ImGui::TableNextRow();
        ImGui::TableNextColumn();

		ImGui::PushID(label);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y * 0.5f);
		ImGui::Text(label);
		if (tooltip && ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text(tooltip);
			ImGui::EndTooltip();
		}

		ImGui::TableNextColumn();

		if (rightAlignNextColumn)
			ImGui::SetNextItemWidth(-FLT_MIN);
	}
	
	void UI::EndPropertyGrid()
	{
		ImGui::PopID();
		PopID();
	}

	void UI::BeginProperties(ImGuiTableFlags flags)
	{
		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);

		ImVec2 cellPadding = ImGui::GetStyle().CellPadding;
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { cellPadding.x * 4, cellPadding.y });

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInner
			| ImGuiTableFlags_BordersOuterH
			| ImGuiTableFlags_PadOuterX;
		ImGui::BeginTable(s_IDBuffer, 2, tableFlags | flags);
	}

	void UI::EndProperties()
	{
		ImGui::EndTable();
		ImGui::PopStyleVar();
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Strings //////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, eastl::string& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		// Small strings
		if (value.size() < 255)
		{
			constexpr size_t size = 256;
			char buffer[size];
			memcpy(buffer, value.data(), size);

			s_IDBuffer[0] = '#';
			s_IDBuffer[1] = '#';
			memset(s_IDBuffer + 2, 0, 14);
			itoa(s_Counter++, s_IDBuffer + 2, 16);
			if (ImGui::InputText(s_IDBuffer, buffer, size))
			{
				value = buffer;
				modified = true;
			}
		}
		// Big strings
		else
		{
			const size_t size = value.size() + 256;
			char* buffer = new char[size];
			memcpy(buffer, value.data(), size);

			s_IDBuffer[0] = '#';
			s_IDBuffer[1] = '#';
			memset(s_IDBuffer + 2, 0, 14);
			itoa(s_Counter++, s_IDBuffer + 2, 16);
			if (ImGui::InputText(s_IDBuffer, buffer, size))
			{
				value = buffer;
				modified = true;
			}

			delete[size] buffer;
		}

		EndPropertyGrid();

		return modified;
	}

	void UI::Property(const char* label, const char* value, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		ImGui::InputText(s_IDBuffer, (char*)value, 256, ImGuiInputTextFlags_ReadOnly);

		EndPropertyGrid();
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// s/byte ///////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, int8_t& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_S8, &value))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, uint8_t& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_U8, &value))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, int8_t& value, int8_t min, int8_t max, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_S8, &value, &min, &max))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, uint8_t& value, uint8_t min, uint8_t max, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_U8, &value, &min, &max))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// u/short //////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, int16_t& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_S16, &value))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, uint16_t& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_U16, &value))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, int16_t& value, int16_t min, int16_t max, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_S16, &value, &min, &max))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, uint16_t& value, uint16_t min, uint16_t max, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_U16, &value, &min, &max))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// u/int ////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, int32_t& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_S32, &value))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, uint32_t& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_U32, &value))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, int32_t& value, int32_t min, int32_t max, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_S32, &value, &min, &max))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, uint32_t& value, uint32_t min, uint32_t max, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_U32, &value, &min, &max))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// u/long ///////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, int64_t& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_S64, &value))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, uint64_t& value, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_U64, &value))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, int64_t& value, int64_t min, int64_t max, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_S64, &value, &min, &max))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, uint64_t& value, uint64_t min, uint64_t max, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_U64, &value, &min, &max))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Float ////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	bool UI::Property(const char* label, float& value, const char* tooltip)
	{
		return Property(label, value, 0.1f, tooltip);
	}

	bool UI::Property(const char* label, float& value, float delta, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_Float, &value, delta))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, float& value, float min, float max, const char* fmt, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_Float, &value, &min, &max, fmt))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, float& value, float delta, float min, float max, const char* fmt, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_Float, &value, delta, &min, &max, fmt))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Double ///////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, double& value, const char* tooltip)
	{
		return Property(label, value, 0.1, tooltip);
	}

	bool UI::Property(const char* label, double& value, float delta, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_Double, &value, delta))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, double& value, float min, float max, const char* fmt, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_Double, &value, &min, &max, fmt))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, double& value, float delta, float min, float max, const char* fmt, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_Double, &value, delta, &min, &max, fmt))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Vec2/3/4 /////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, glm::vec2& value, const char* tooltip)
	{
		return Property(label, value, 0.1f, tooltip);
	}

	bool UI::Property(const char* label, glm::vec3& value, const char* tooltip)
	{
		return Property(label, value, 0.1f, tooltip);
	}

	bool UI::Property(const char* label, glm::vec4& value, const char* tooltip)
	{
		return Property(label, value, 0.1f, tooltip);
	}

	bool UI::Property(const char* label, glm::vec2& value, float delta, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat2(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, glm::vec3& value, float delta, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat3(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::Property(const char* label, glm::vec4& value, float delta, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat4(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Bool /////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, bool& flag, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::Checkbox(s_IDBuffer, &flag))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Dropdown /////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, int& value, const char** dropdownStrings, size_t count, const char* tooltip)
	{
		bool modified = false;

		const char* current = dropdownStrings[(int)value];
		
		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);

		if(ImGui::BeginCombo(s_IDBuffer, current))
		{
			for (int i = 0; i < count; i++)
			{
				bool isSelected = current == dropdownStrings[i];
				if(ImGui::Selectable(dropdownStrings[i], isSelected))
				{
					current = dropdownStrings[i];
					value = i;
					modified = true;
				}

				if(isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		UI::EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Colors ///////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::PropertyColor3(const char* label, glm::vec3& color, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::ColorEdit3(s_IDBuffer, glm::value_ptr(color)))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::PropertyColor4(const char* label, glm::vec4& color, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::ColorEdit4(s_IDBuffer, glm::value_ptr(color)))
			modified = true;

		EndPropertyGrid();

		return modified;
	}

	bool UI::PropertyColor4as3(const char* label, glm::vec4& color, const char* tooltip)
	{
		bool modified = false;

		BeginPropertyGrid(label, tooltip);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::ColorEdit3(s_IDBuffer, glm::value_ptr(color)))
			modified = true;

		EndPropertyGrid();

		return modified;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// 2D/3D Textures ///////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, Ref<TextureCubemap>& texture, uint32_t overrideTextureID, const char* tooltip)
	{
		bool changed = false;

		const ImVec2 buttonSize = { 80, 80 };
		const ImVec2 xButtonSize = { buttonSize.x / 4.0f, 80 };
		const ImVec2 tooltipSize = { 300, 300 };

		BeginPropertyGrid(label, tooltip, buttonSize.y);

		ImGui::SetCursorPos({ ImGui::GetContentRegionMax().x - buttonSize.x - xButtonSize.x, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });

		uint32_t id = overrideTextureID;
		if (id == 0)
			id = texture == nullptr ? 0 : texture->GetHRDRendererID();
		ImGui::ImageButton((ImTextureID)id, buttonSize, { 1, 1 }, { 0, 0 }, 0);
		if (texture && ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text(texture->GetPath().c_str());
			ImGui::Spacing();
			ImGui::Image((ImTextureID)id, tooltipSize, { 1, 1 }, { 0, 0 });
			ImGui::EndTooltip();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				texture = AssetManager::GetTextureCubemap(path);
				changed = true;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		if(ImGui::Button("x", xButtonSize))
		{
			texture = nullptr;
			changed = true;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		EndPropertyGrid();

		return changed;
	}

	bool UI::Property(const char* label, Ref<Texture2D>& texture, uint32_t overrideTextureID, const char* tooltip)
	{
		bool changed = false;

		const ImVec2 buttonSize = { 80, 80 };
		const ImVec2 xButtonSize = { buttonSize.x / 4.0f, 80 };
		const ImVec2 tooltipSize = { 300, 300 };

		BeginPropertyGrid(label, tooltip, buttonSize.y);

		ImGui::SetCursorPos({ ImGui::GetContentRegionMax().x - buttonSize.x - xButtonSize.x, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		
		uint32_t id = overrideTextureID;
		if (id == 0)
			id = texture == nullptr ? 0 : texture->GetRendererID();
		ImGui::ImageButton((ImTextureID)id, buttonSize, { 1, 1 }, { 0, 0 }, 0);
		if (texture && ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text(texture->GetPath().c_str());
			ImGui::Spacing();
			ImGui::Image((ImTextureID)id, tooltipSize, { 1, 1 }, { 0, 0 });
			ImGui::EndTooltip();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				texture = AssetManager::GetTexture2D(path);
				changed = true;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		if(ImGui::Button("x", xButtonSize ))
		{
			texture = nullptr;
			changed = true;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		EndPropertyGrid();

		return changed;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Vec3 with reset button ///////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	void UI::DrawVec3Control(const char* label, glm::vec3& values, float resetValue, float columnWidth, const char* tooltip)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		
		BeginPropertyGrid(label, tooltip, false);

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImVec2 innerItemSpacing = ImGui::GetStyle().ItemInnerSpacing;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, innerItemSpacing);

		// X
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize))
				values.x = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::PopStyleVar();
		}

		ImGui::SameLine();

		// Y
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
				values.y = resetValue;
			ImGui::PopFont();

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::PopStyleVar();
		}

		ImGui::SameLine();

		// Z
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize))
				values.z = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::PopStyleVar();
		}

		ImGui::PopStyleVar();
		
		EndPropertyGrid();
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Buttons //////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::IconButton(const char* icon, const char* label, ImVec4 iconColor)
	{
		bool clicked = false;

		float lineHeight = ImGui::GetTextLineHeight();
		ImVec2 padding = ImGui::GetStyle().FramePadding;

		float width = ImGui::CalcTextSize(icon).x;
		width += ImGui::CalcTextSize(label).x;
		width += padding.x * 2.0f;

		ImVec2 buttonSize = { width, lineHeight + padding.y * 2.0f };

		const float cursorPosX = ImGui::GetCursorPosX();
		if (ImGui::Button((eastl::string("##") + label).c_str(), buttonSize))
			clicked = true;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
		ImGui::SameLine();
		ImGui::SetCursorPosX(cursorPosX);
		ImGui::TextColored(iconColor, icon);
		ImGui::SameLine();
		ImGui::Text(label);
		ImGui::PopStyleVar();

		return clicked;
	}

	bool UI::ToggleButton(const char* label, bool state, ImVec2 size, float alpha, float pressedAlpha, ImGuiButtonFlags buttonFlags)
	{
		if (state)
		{
			ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

			color.w = pressedAlpha;
			ImGui::PushStyleColor(ImGuiCol_Button, color);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
		}
		else
		{
			ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_Button];
			ImVec4 hoveredColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
			color.w = alpha;
			hoveredColor.w = pressedAlpha;
			ImGui::PushStyleColor(ImGuiCol_Button, color);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
			color.w = pressedAlpha;
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
		}

		bool clicked = ImGui::ButtonEx(label, size, buttonFlags);

		ImGui::PopStyleColor(3);

		return clicked;
	}
}
