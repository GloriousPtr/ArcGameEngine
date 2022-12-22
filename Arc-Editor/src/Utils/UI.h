#pragma once

#include <ArcEngine.h>

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

struct ImRect;

namespace ArcEngine
{
	class UI
	{
	public:
		static void BeginProperties(ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInner
			| ImGuiTableFlags_BordersOuterH);
		static void EndProperties();

		// Strings
		static bool Property(const char* label, std::string& value, const char* tooltip = nullptr);

		template<std::integral T>
		static bool Property(const char* label, T& value, T min = 0, T max = 0, const char* tooltip = nullptr)
		{
			BeginPropertyGrid(label, tooltip);
			bool modified;

			int dataType = ImGuiDataType_S32;
			if constexpr (std::is_signed_v<T>)
			{
				if constexpr (sizeof(T) == 1)
					dataType = ImGuiDataType_S8;
				else if constexpr (sizeof(T) == 2)
					dataType = ImGuiDataType_S16;
				else if constexpr (sizeof(T) == 4)
					dataType = ImGuiDataType_S32;
				else if constexpr (sizeof(T) == 8)
					dataType = ImGuiDataType_S64;
			}
			else
			{
				if constexpr (sizeof(T) == 1)
					dataType = ImGuiDataType_U8;
				else if constexpr (sizeof(T) == 2)
					dataType = ImGuiDataType_U16;
				else if constexpr (sizeof(T) == 4)
					dataType = ImGuiDataType_U32;
				else if constexpr (sizeof(T) == 8)
					dataType = ImGuiDataType_U64;
			}

			if (max > min)
				modified = ImGui::SliderScalar(s_IDBuffer, dataType, &value, &min, &max);
			else
				modified = ImGui::DragScalar(s_IDBuffer, dataType, &value);

			EndPropertyGrid();
			return modified;
		}

		template<std::floating_point T>
		static bool Property(const char* label, T& value, T min = 0, T max = 0, const char* tooltip = nullptr, float delta = 0.1f, const char* fmt = "%.3f")
		{
			BeginPropertyGrid(label, tooltip);
			bool modified;

			int dataType = ImGuiDataType_Float;
			if constexpr (sizeof(T) == 8)
				dataType = ImGuiDataType_Double;

			if (max > min)
				modified = ImGui::SliderScalar(s_IDBuffer, dataType, &value, &min, &max, fmt);
			else
				modified = ImGui::DragScalar(s_IDBuffer, dataType, &value, delta, nullptr, nullptr, fmt);

			EndPropertyGrid();
			return modified;
		}

		// Vec2/3/4
		template<typename T>
		static bool PropertyVector(const char* label, T& value, bool color = false, bool showAlpha = true, const char* tooltip = nullptr, float delta = 0.1f)
		{
			BeginPropertyGrid(label, tooltip);
			bool modified;
			int componentCount = value.length();
			if (componentCount >= 3 && color)
			{
				if (showAlpha)
					modified = ImGui::ColorEdit4(s_IDBuffer, glm::value_ptr(value));
				else
					modified = ImGui::ColorEdit3(s_IDBuffer, glm::value_ptr(value));
			}
			else
			{
				modified = ImGui::DragScalarN(s_IDBuffer, ImGuiDataType_Float, glm::value_ptr(value), componentCount, delta);
			}
			EndPropertyGrid();
			return modified;
		}

		// Bool
		static bool Property(const char* label, bool& flag, const char* tooltip = nullptr);

		// Dropdown
		static bool Property(const char* label, int& value, const char** dropdownStrings, size_t count, const char* tooltip = nullptr);

		// 2D/3D Textures
		static bool Property(const char* label, Ref<TextureCubemap>& texture, uint64_t overrideTextureID = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, Ref<Texture2D>& texture, uint64_t overrideTextureID = 0, const char* tooltip = nullptr);

		template<typename T, typename Fn>
			requires std::integral<T> || std::floating_point<T>
		bool Property(const char* label, std::vector<T>&v, T defaultValue = 0, size_t minElements = 0, const char* tooltip = nullptr)
		{
			return ListProperty(label, v, defaultValue, minElements, tooltip, [](const char* name, int8_t& value) { UI::Property(name, value); });
		}

		// Vec2/3/4
		static bool Property(const char* label, std::vector<glm::vec2>& v, const glm::vec2& defaultValue = glm::vec2(0.0f), size_t minElements = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, std::vector<glm::vec3>& v, const glm::vec3& defaultValue = glm::vec3(0.0f), size_t minElements = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, std::vector<glm::vec4>& v, const glm::vec4& defaultValue = glm::vec4(0.0f), size_t minElements = 0, const char* tooltip = nullptr);

		// Colors
		static bool PropertyColor(const char* label, std::vector<glm::vec3>& v, const glm::vec3& defaultValue = glm::vec3(1.0f), size_t minElements = 0, const char* tooltip = nullptr);
		static bool PropertyColor(const char* label, std::vector<glm::vec4>& v, const glm::vec4& defaultValue = glm::vec4(1.0f), size_t minElements = 0, const char* tooltip = nullptr);
		static bool PropertyColor4as3(const char* label, std::vector<glm::vec4>& v, const glm::vec4& defaultValue = glm::vec4(1.0f), size_t minElements = 0, const char* tooltip = nullptr);




		template<typename T>
		static bool PropertyComponent(const char* label, const char* text, Scene* scene, UUID& entityID, const char* tooltip = nullptr)
		{
			Entity entity;
			if (entityID)
				entity = scene->GetEntity(entityID);

			const char* tag = "none";
			if (entity && entity.HasComponent<T>())
				tag = entity.GetComponent<TagComponent>().Tag.c_str();
			else
				entityID = 0;

			BeginPropertyGrid(label, tooltip);

			bool modified = false;

			ImVec2 region = ImGui::GetContentRegionAvail();
			region.x -= 20.0f;
			region.y = ImGui::GetFrameHeight();

			ImVec2 pos = ImGui::GetCursorPos();
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_Button]);
			ImGui::Button(s_IDBuffer, region);
			ImGui::PopStyleColor();
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					Entity* payloadEntity = (Entity*)payload->Data;
					if (payloadEntity->HasComponent<T>())
					{
						entityID = payloadEntity->GetUUID();
						modified = true;
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
			if (ImGui::Button("x", { 20.0f, region.y }))
			{
				entityID = 0;
				modified = true;
			}
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			if (!entityID)
				ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.2f, 0.2f, 1.0f });

			ImVec2 padding = ImGui::GetStyle().FramePadding;
			ImGui::SetCursorPos({ pos.x + padding.x, pos.y + padding.y });
			ImGui::Text("%s (%s)", tag, text);

			if (!entityID)
				ImGui::PopStyleColor();

			EndPropertyGrid();

			return modified;
		}

		// Field
		static void DrawField(Entity entity, const std::string& className, const std::string& fieldName);

		// Vec3 with reset button
		static void DrawVec3Control(const char* label, glm::vec3& values, const char* tooltip = nullptr, float resetValue = 0.0f);

		// Buttons
		static glm::vec2 GetIconButtonSize(const char8_t* icon, const char* label);
		static bool IconButton(const char8_t* icon, const char* label, ImVec4 iconColor = { 0.537f, 0.753f, 0.286f, 1.0f });
		static bool ToggleButton(const char* label, bool state, ImVec2 size = { 0, 0 }, float alpha = 1.0f, float pressedAlpha = 1.0f, ImGuiButtonFlags buttonFlags = ImGuiButtonFlags_None);

		static void ClippedText(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect, float wrap_width);
		static void ClippedText(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_display_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect, float wrap_width);
		static void AddTextVertical(ImDrawList* DrawList, const char* text, ImVec2 pos, ImU32 text_color);

	private:
		static void PushID();
		static void PopID();

		static void BeginPropertyGrid(const char* label, const char* tooltip, bool rightAlignNextColumn = true);
		static void EndPropertyGrid();

		static void BeginProperties3(ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInner
			| ImGuiTableFlags_BordersOuterH);

		template<typename T, typename Fn>
		static bool ListProperty(const char* label, std::vector<T>& v, const T& defaultValue, size_t minElements, const char* tooltip, Fn function);

	private:
		static int s_UIContextID;
		static uint32_t s_Counter;
		static char s_IDBuffer[16];
	};
}
