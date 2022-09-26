#include "UI.h"

#include <glm/gtc/type_ptr.hpp>
#include <icons/IconsMaterialDesignIcons.h>

namespace ArcEngine
{
	int UI::s_UIContextID = 0;
	uint32_t UI::s_Counter = 0;
	char UI::s_IDBuffer[16];

	void UI::PushID()
	{
		++s_UIContextID;
		ImGui::PushID(s_UIContextID);
		s_Counter = 0;
	}

	void UI::PopID()
	{
		ImGui::PopID();
		--s_UIContextID;
	}
	
	void UI::BeginPropertyGrid(const char* label, const char* tooltip, bool rightAlignNextColumn)
	{
		PushID();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::PushID(label);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y * 0.5f);
		ImGui::Text(label);
		if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
		{
			ImGui::BeginTooltip();
			ImGui::Text(tooltip);
			ImGui::EndTooltip();
		}

		ImGui::TableNextColumn();

		if (rightAlignNextColumn)
			ImGui::SetNextItemWidth(-FLT_MIN);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		++s_Counter;
		_itoa_s(s_Counter, s_IDBuffer + 2, 16, 16);
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
		++s_Counter;
		_itoa_s(s_Counter, s_IDBuffer + 2, 16, 16);

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_PadOuterX;
		ImGui::BeginTable(s_IDBuffer, 2, tableFlags | flags);
		ImGui::TableSetupColumn("PropertyName",0 , 0.5f);
		ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch);
	}

	void UI::BeginProperties3(ImGuiTableFlags flags)
	{
		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		++s_Counter;
		_itoa_s(s_Counter, s_IDBuffer + 2, 16, 16);

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_PadOuterX;
		ImGui::BeginTable(s_IDBuffer, 3, tableFlags | flags);
		ImGui::TableSetupColumn("PropertyName", 0, 0.5f);
		ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("PropertyExtra", 0, 0.1f);
	}
	
	void UI::EndProperties()
	{
		ImGui::EndTable();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Strings //////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, eastl::string& value, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);

		bool modified = false;

		// Small strings
		if (value.size() < 255)
		{
			constexpr size_t size = 256;
			char buffer[size];
			memcpy(buffer, value.data(), size);

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

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// s/byte ///////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, int8_t& value, int8_t min, int8_t max, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_S8, &value, &min, &max))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_S8, &value))
				modified = true;
		}

		EndPropertyGrid();
		return modified;
	}

	bool UI::Property(const char* label, uint8_t& value, uint8_t min, uint8_t max, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_U8, &value, &min, &max))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_U8, &value))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// u/short //////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, int16_t& value, int16_t min, int16_t max, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_S16, &value, &min, &max))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_S16, &value))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	bool UI::Property(const char* label, uint16_t& value, uint16_t min, uint16_t max, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_U16, &value, &min, &max))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_U16, &value))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// u/int ////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, int32_t& value, int32_t min, int32_t max, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_S32, &value, &min, &max))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_S32, &value))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	bool UI::Property(const char* label, uint32_t& value, uint32_t min, uint32_t max, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_U32, &value, &min, &max))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_U32, &value))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// u/long ///////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, int64_t& value, int64_t min, int64_t max, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_S64, &value, &min, &max))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_S64, &value))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	bool UI::Property(const char* label, uint64_t& value, uint64_t min, uint64_t max, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_U64, &value, &min, &max))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_U64, &value))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Float ////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, float& value, float min, float max, const char* tooltip, float delta, const char* fmt)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_Float, &value, &min, &max, fmt))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_Float, &value, delta, nullptr, nullptr, fmt))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Double ///////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, double& value, double min, double max, const char* tooltip, float delta, const char* fmt)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (max > min)
		{
			if (ImGui::SliderScalar(s_IDBuffer, ImGuiDataType_Double, &value, &min, &max, fmt))
				modified = true;
		}
		else
		{
			if (ImGui::DragScalar(s_IDBuffer, ImGuiDataType_Double, &value, delta, nullptr, nullptr, fmt))
				modified = true;
		}
		EndPropertyGrid();
		return modified;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Vec2/3/4 /////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	bool UI::Property(const char* label, glm::vec2& value, const char* tooltip, float delta)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (ImGui::DragFloat2(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;
		EndPropertyGrid();
		return modified;
	}

	bool UI::Property(const char* label, glm::vec3& value, const char* tooltip, float delta)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (ImGui::DragFloat3(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;
		EndPropertyGrid();
		return modified;
	}

	bool UI::Property(const char* label, glm::vec4& value, const char* tooltip, float delta)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
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
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
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
		BeginPropertyGrid(label, tooltip);

		bool modified = false;
		const char* current = dropdownStrings[value];

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
	bool UI::PropertyColor(const char* label, glm::vec3& color, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (ImGui::ColorEdit3(s_IDBuffer, glm::value_ptr(color)))
			modified = true;
		EndPropertyGrid();
		return modified;
	}

	bool UI::PropertyColor(const char* label, glm::vec4& color, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (ImGui::ColorEdit4(s_IDBuffer, glm::value_ptr(color)))
			modified = true;
		EndPropertyGrid();
		return modified;
	}

	bool UI::PropertyColor4as3(const char* label, glm::vec4& color, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);
		bool modified = false;
		if (ImGui::ColorEdit3(s_IDBuffer, glm::value_ptr(color)))
			modified = true;
		EndPropertyGrid();
		return modified;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// 2D/3D Textures ///////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	bool UI::Property(const char* label, Ref<TextureCubemap>& texture, uint64_t overrideTextureID, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);

		bool changed = false;

		float frameHeight = ImGui::GetFrameHeight();
		const float buttonSize = frameHeight * 3.0f;
		const ImVec2 xButtonSize = { buttonSize / 4.0f, buttonSize };
		const float tooltipSize = frameHeight * 11.0f;

		ImGui::SetCursorPos({ ImGui::GetContentRegionMax().x - buttonSize - xButtonSize.x, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });

		uint64_t id = overrideTextureID;
		if (id == 0)
			id = texture == nullptr ? 0 : texture->GetHRDRendererID();
		ImGui::ImageButton((ImTextureID)id, { buttonSize, buttonSize }, { 1, 1 }, { 0, 0 }, 0);
		if (texture && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
		{
			ImGui::BeginTooltip();
			ImGui::Text(texture->GetPath().c_str());
			ImGui::Spacing();
			ImGui::Image((ImTextureID)id, { tooltipSize, tooltipSize }, { 1, 1 }, { 0, 0 });
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

	bool UI::Property(const char* label, Ref<Texture2D>& texture, uint64_t overrideTextureID, const char* tooltip)
	{
		BeginPropertyGrid(label, tooltip);

		bool changed = false;

		float frameHeight = ImGui::GetFrameHeight();
		const float buttonSize = frameHeight * 3.0f;
		const ImVec2 xButtonSize = { buttonSize / 4.0f, buttonSize };
		const float tooltipSize = frameHeight * 11.0f;

		ImGui::SetCursorPos({ ImGui::GetContentRegionMax().x - buttonSize - xButtonSize.x, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		
		uint64_t id = overrideTextureID;
		if (id == 0)
			id = texture == nullptr ? 0 : texture->GetRendererID();
		ImGui::ImageButton((ImTextureID)id, { buttonSize, buttonSize }, { 1, 1 }, { 0, 0 }, 0);
		if (texture && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
		{
			ImGui::BeginTooltip();
			ImGui::Text(texture->GetPath().c_str());
			ImGui::Spacing();
			ImGui::Image((ImTextureID)id, { tooltipSize, tooltipSize }, { 1, 1 }, { 0, 0 });
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



	template<typename T, typename Fn>
	bool UI::ListProperty(const char* label, eastl::vector<T>& v, const T& defaultValue, size_t minElements, const char* tooltip, Fn function)
	{
		bool modified = false;
		if (ImGui::TreeNode(label))
		{
			BeginProperties3();

			size_t pointsCount = v.size();
			size_t step = 1u;
			BeginPropertyGrid("Count", tooltip);
			ImGui::InputScalar(s_IDBuffer, ImGuiDataType_U64, &pointsCount, &step);
			ImGui::TableNextColumn();
			EndPropertyGrid();

			if (pointsCount > v.size())
			{
				v.emplace_back(defaultValue);
				modified = true;
			}
			else if (pointsCount < v.size() && minElements < v.size())
			{
				v.erase(v.end() - 1);
				modified = true;
			}

			uint32_t i = 0;
			std::string name;
			T* removeIt = nullptr;
			for (auto* it = v.begin(); it != v.end(); ++it)
			{
				ImGui::PushID(it);
				name = fmt::format("Point {}", i);
				function(name.c_str(), v[i]);
				ImGui::TableNextColumn();

				bool disabled = pointsCount <= minElements;
				if (disabled)
					ImGui::BeginDisabled();

				if (ImGui::Button(ICON_MDI_CLOSE))
					removeIt = it;

				ImGui::PopID();

				if (disabled)
					ImGui::EndDisabled();

				++i;
			}
			EndProperties();

			if (removeIt)
				v.erase(removeIt);

			ImGui::TreePop();
		}

		return modified;
	}

	bool UI::Property(const char* label, eastl::vector<int8_t>& v, int8_t defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, int8_t& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<uint8_t>& v, uint8_t defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, uint8_t& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<int16_t>& v, int16_t defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, int16_t& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<uint16_t>& v, uint16_t defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, uint16_t& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<int32_t>& v, int32_t defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, int32_t& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<uint32_t>& v, uint32_t defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, uint32_t& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<int64_t>& v, int64_t defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, int64_t& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<uint64_t>& v, uint64_t defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, uint64_t& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<float>& v, float defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, float& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<double>& v, double defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, double& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<glm::vec2>& v, const glm::vec2& defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, glm::vec2& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<glm::vec3>& v, const glm::vec3& defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, glm::vec3& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<glm::vec4>& v, const glm::vec4& defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, glm::vec4& value) { UI::Property(name, value); });
	}

	bool UI::Property(const char* label, eastl::vector<bool>& v, bool defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, bool& value) { UI::Property(name, value); });
	}

	bool UI::PropertyColor(const char* label, eastl::vector<glm::vec3>& v, const glm::vec3& defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, glm::vec3& value) { UI::PropertyColor(name, value); });
	}

	bool UI::PropertyColor(const char* label, eastl::vector<glm::vec4>& v, const glm::vec4& defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, glm::vec4& value) { UI::PropertyColor(name, value); });
	}

	bool UI::PropertyColor4as3(const char* label, eastl::vector<glm::vec4>& v, const glm::vec4& defaultValue, size_t minElements, const char* tooltip)
	{
		return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, glm::vec4& value) { UI::PropertyColor4as3(name, value); });
	}







	template<typename T>
	static void DrawScriptFieldScalar(Entity entity, const eastl::string& className, const eastl::string& fieldName)
	{
		const auto& fieldMap = ScriptEngine::GetFieldMap(className.c_str());
		const ScriptField& field = fieldMap.at(fieldName);
		const char* tooltip = field.Tooltip.empty() ? nullptr : field.Tooltip.c_str();
		T min = (T)field.Min;
		T max = (T)field.Max;

		ScriptInstance* scriptInstance = (ScriptEngine::HasInstance(entity, className) ? ScriptEngine::GetInstance(entity, className) : nullptr);
		if (!scriptInstance)
		{
			auto& fieldInstanceMap = ScriptEngine::GetFieldInstanceMap(entity, className.c_str());
			if (fieldInstanceMap.find_as(fieldName) != fieldInstanceMap.end())
			{
				ScriptFieldInstance& fieldInstance = fieldInstanceMap.at(fieldName);

				if (fieldInstance.Type != field.Type)
				{
					fieldInstanceMap.erase(fieldName);
					return;
				}

				T value = fieldInstance.GetValue<T>();
				if (UI::Property(field.Name.c_str(), value, min, max, tooltip))
					fieldInstance.SetValue(value);
			}
			else
			{
				T value = field.GetDefaultValue<T>();
				if (UI::Property(field.Name.c_str(), value, min, max, tooltip))
				{
					fieldInstanceMap[fieldName].Type = field.Type;
					fieldInstanceMap[fieldName].SetValue(value);
				}
			}
		}
		else
		{
			T value = scriptInstance->GetFieldValue<T>(field.Name);
			if (UI::Property(field.Name.c_str(), value, min, max, tooltip))
				scriptInstance->SetFieldValue<T>(field.Name, value);
		}
	}

	template<typename T>
	static void DrawScriptField(Entity entity, const eastl::string& className, const eastl::string& fieldName)
	{
		const auto& fieldMap = ScriptEngine::GetFieldMap(className.c_str());
		const ScriptField& field = fieldMap.at(fieldName);
		const char* tooltip = field.Tooltip.empty() ? nullptr : field.Tooltip.c_str();

		ScriptInstance* scriptInstance = (ScriptEngine::HasInstance(entity, className) ? ScriptEngine::GetInstance(entity, className) : nullptr);
		if (!scriptInstance)
		{
			auto& fieldInstanceMap = ScriptEngine::GetFieldInstanceMap(entity, className.c_str());
			if (fieldInstanceMap.find_as(fieldName) != fieldInstanceMap.end())
			{
				ScriptFieldInstance& fieldInstance = fieldInstanceMap.at(fieldName);

				if (fieldInstance.Type != field.Type)
				{
					fieldInstanceMap.erase(fieldName);
					return;
				}

				T value = fieldInstance.GetValue<T>();
				if (UI::Property(field.Name.c_str(), value, tooltip))
					fieldInstance.SetValue(value);
			}
			else
			{
				T value = field.GetDefaultValue<T>();
				if (UI::Property(field.Name.c_str(), value, tooltip))
				{
					fieldInstanceMap[fieldName].Type = field.Type;
					fieldInstanceMap[fieldName].SetValue(value);
				}
			}
		}
		else
		{
			T value = scriptInstance->GetFieldValue<T>(field.Name);
			if (UI::Property(field.Name.c_str(), value, tooltip))
				scriptInstance->SetFieldValue<T>(field.Name, value);
		}
	}

	template<typename T>
	static void DrawScriptFieldColor(Entity entity, const eastl::string& className, const eastl::string& fieldName)
	{
		const auto& fieldMap = ScriptEngine::GetFieldMap(className.c_str());
		const ScriptField& field = fieldMap.at(fieldName);
		const char* tooltip = field.Tooltip.empty() ? nullptr : field.Tooltip.c_str();

		ScriptInstance* scriptInstance = (ScriptEngine::HasInstance(entity, className) ? ScriptEngine::GetInstance(entity, className) : nullptr);
		if (!scriptInstance)
		{
			auto& fieldInstanceMap = ScriptEngine::GetFieldInstanceMap(entity, className.c_str());
			if (fieldInstanceMap.find_as(fieldName) != fieldInstanceMap.end())
			{
				ScriptFieldInstance& fieldInstance = fieldInstanceMap.at(fieldName);

				if (fieldInstance.Type != field.Type)
				{
					fieldInstanceMap.erase(fieldName);
					return;
				}

				T value = fieldInstance.GetValue<T>();
				if (UI::PropertyColor(field.Name.c_str(), value, tooltip))
					fieldInstance.SetValue(value);
			}
			else
			{
				T value = field.GetDefaultValue<T>();
				if (UI::PropertyColor(field.Name.c_str(), value, tooltip))
				{
					fieldInstanceMap[fieldName].Type = field.Type;
					fieldInstanceMap[fieldName].SetValue(value);
				}
			}
		}
		else
		{
			T value = scriptInstance->GetFieldValue<T>(field.Name);
			if (UI::PropertyColor(field.Name.c_str(), value, tooltip))
				scriptInstance->SetFieldValue<T>(field.Name, value);
		}
	}

	static void DrawScriptFieldString(Entity entity, const eastl::string& className, const eastl::string& fieldName)
	{
		const auto& fieldMap = ScriptEngine::GetFieldMap(className.c_str());
		const ScriptField& field = fieldMap.at(fieldName);
		const char* tooltip = field.Tooltip.empty() ? nullptr : field.Tooltip.c_str();

		ScriptInstance* scriptInstance = (ScriptEngine::HasInstance(entity, className) ? ScriptEngine::GetInstance(entity, className) : nullptr);
		if (!scriptInstance)
		{
			auto& fieldInstanceMap = ScriptEngine::GetFieldInstanceMap(entity, className.c_str());
			if (fieldInstanceMap.find_as(fieldName) != fieldInstanceMap.end())
			{
				ScriptFieldInstance& fieldInstance = fieldInstanceMap.at(fieldName);

				if (fieldInstance.Type != field.Type)
				{
					fieldInstanceMap.erase(fieldName);
					return;
				}

				eastl::string value = (const char*)fieldInstance.GetBuffer();
				if (UI::Property(field.Name.c_str(), value, tooltip))
					fieldInstance.SetValueString(value.c_str());
			}
			else
			{
				eastl::string value = field.DefaultValue;
				if (UI::Property(field.Name.c_str(), value, tooltip))
				{
					fieldInstanceMap[fieldName].Type = field.Type;
					fieldInstanceMap[fieldName].SetValueString(value.c_str());
				}
			}
		}
		else
		{
			eastl::string value = scriptInstance->GetFieldValueString(field.Name);
			if (UI::Property(field.Name.c_str(), value, tooltip))
				scriptInstance->SetFieldValue<const char*>(field.Name, value.c_str());
		}
	}

	void UI::DrawField(Entity entity, const eastl::string& className, const eastl::string& fieldName)
	{
		const ScriptField& field = ScriptEngine::GetFieldMap(className.c_str()).at(fieldName);
		if (field.Type == FieldType::Unknown)
			return;

		if (field.Hidden)
			return;

		switch (field.Type)
		{
			case FieldType::Bool:
			{
				DrawScriptField<bool>(entity, className, fieldName);
				break;
			}
			case FieldType::Float:
			{
				DrawScriptFieldScalar<float>(entity, className, fieldName);
				break;
			}
			case FieldType::Double:
			{
				DrawScriptFieldScalar<double>(entity, className, fieldName);
				break;
			}
			case FieldType::Byte:
			{
				DrawScriptFieldScalar<int8_t>(entity, className, fieldName);
				break;
			}
			case FieldType::UByte:
			{
				DrawScriptFieldScalar<uint8_t>(entity, className, fieldName);
				break;
			}
			case FieldType::Short:
			{
				DrawScriptFieldScalar<int16_t>(entity, className, fieldName);
				break;
			}
			case FieldType::UShort:
			{
				DrawScriptFieldScalar<uint16_t>(entity, className, fieldName);
				break;
			}
			case FieldType::Int:
			{
				DrawScriptFieldScalar<int32_t>(entity, className, fieldName);
				break;
			}
			case FieldType::UInt:
			{
				DrawScriptFieldScalar<uint32_t>(entity, className, fieldName);
				break;
			}
			case FieldType::Long:
			{
				DrawScriptFieldScalar<int64_t>(entity, className, fieldName);
				break;
			}
			case FieldType::ULong:
			{
				DrawScriptFieldScalar<uint64_t>(entity, className, fieldName);
				break;
			}
			case FieldType::String:
			{
				DrawScriptFieldString(entity, className, fieldName);
				break;
			}
			case FieldType::Vector2:
			{
				DrawScriptField<glm::vec2>(entity, className, fieldName);
				break;
			}
			case FieldType::Vector3:
			{
				DrawScriptField<glm::vec3>(entity, className, fieldName);
				break;
			}
			case FieldType::Vector4:
			{
				DrawScriptField<glm::vec4>(entity, className, fieldName);
				break;
			}
			case FieldType::Color:
			{
				DrawScriptFieldColor<glm::vec4>(entity, className, fieldName);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// Vec3 with reset button ///////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	void UI::DrawVec3Control(const char* label, glm::vec3& values, const char* tooltip, float resetValue)
	{
		BeginPropertyGrid(label, tooltip, false);

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

		float frameHeight = ImGui::GetFrameHeight();
		ImVec2 buttonSize = { frameHeight + 3.0f, frameHeight };

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

	glm::vec2 UI::GetIconButtonSize(const char* icon, const char* label)
	{
		float lineHeight = ImGui::GetTextLineHeight();
		ImVec2 padding = ImGui::GetStyle().FramePadding;

		float width = ImGui::CalcTextSize(icon).x;
		width += ImGui::CalcTextSize(label).x;
		width += padding.x * 2.0f;

		return { width, lineHeight + padding.y * 2.0f };
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

	void UI::ClippedText(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect, float wrap_width)
	{
		// Hide anything after a '##' string
		const char* text_display_end = ImGui::FindRenderedTextEnd(text, text_end);
		const int text_len = (int)(text_display_end - text);
		if (text_len == 0)
			return;

		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		UI::ClippedText(window->DrawList, pos_min, pos_max, text, text_display_end, text_size_if_known, align, clip_rect, wrap_width);
		if (g.LogEnabled)
			ImGui::LogRenderedText(&pos_min, text, text_display_end);
	}

	void UI::ClippedText(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_display_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect, float wrap_width)
	{
		// Perform CPU side clipping for single clipped element to avoid using scissor state
		ImVec2 pos = pos_min;
		const ImVec2 text_size = text_size_if_known ? *text_size_if_known : ImGui::CalcTextSize(text, text_display_end, false, wrap_width);

		const ImVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
		const ImVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
		bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
		if (clip_rect) // If we had no explicit clipping rectangle then pos==clip_min
			need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);
		// Align whole block. We should defer that to the better rendering function when we'll have support for individual line alignment.
		if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
		if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);
		// Render
		if (need_clipping)
		{
			ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
			draw_list->AddText(nullptr, 0.0f, pos, ImGui::GetColorU32(ImGuiCol_Text), text, text_display_end, wrap_width, &fine_clip_rect);
		}
		else
		{
			draw_list->AddText(nullptr, 0.0f, pos, ImGui::GetColorU32(ImGuiCol_Text), text, text_display_end, wrap_width, nullptr);
		}
	}
}
