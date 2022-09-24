#pragma once

#include <ArcEngine.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ArcEngine
{
	class UI
	{
	public:
		static void BeginProperties(ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInner
			| ImGuiTableFlags_BordersOuterH);
		static void EndProperties();

		// Strings
		static bool Property(const char* label, eastl::string& value, const char* tooltip = nullptr);

		// s/byte
		static bool Property(const char* label, int8_t& value, int8_t min = 0, int8_t max = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, uint8_t& value, uint8_t min = 0, uint8_t max = 0, const char* tooltip = nullptr);

		// u/short
		static bool Property(const char* label, int16_t& value, int16_t min = 0, int16_t max = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, uint16_t& value, uint16_t min = 0, uint16_t max = 0, const char* tooltip = nullptr);
		
		// u/int
		static bool Property(const char* label, int32_t& value, int32_t min = 0, int32_t max = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, uint32_t& value, uint32_t min = 0, uint32_t max = 0, const char* tooltip = nullptr);

		// u/long
		static bool Property(const char* label, int64_t& value, int64_t min = 0, int64_t max = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, uint64_t& value, uint64_t min = 0, uint64_t max = 0, const char* tooltip = nullptr);

		// Float
		static bool Property(const char* label, float& value, float min = 0.0f, float max = 0.0f, const char* tooltip = nullptr, float delta = 0.1f, const char* fmt = "%.3f");

		// Double
		static bool Property(const char* label, double& value, double min = 0.0, double max = 0.0, const char* tooltip = nullptr, float delta = 0.1f, const char* fmt = "%.6f");

		// Vec2/3/4
		static bool Property(const char* label, glm::vec2& value, const char* tooltip = nullptr, float delta = 0.1f);
		static bool Property(const char* label, glm::vec3& value, const char* tooltip = nullptr, float delta = 0.1f);
		static bool Property(const char* label, glm::vec4& value, const char* tooltip = nullptr, float delta = 0.1f);

		// Bool
		static bool Property(const char* label, bool& flag, const char* tooltip = nullptr);

		// Dropdown
		static bool Property(const char* label, int& value, const char** dropdownStrings, size_t count, const char* tooltip = nullptr);

		// Colors
		static bool PropertyColor3(const char* label, glm::vec3& color, const char* tooltip = nullptr);
		static bool PropertyColor4(const char* label, glm::vec4& color, const char* tooltip = nullptr);
		static bool PropertyColor4as3(const char* label, glm::vec4& color, const char* tooltip = nullptr);

		// 2D/3D Textures
		static bool Property(const char* label, Ref<TextureCubemap>& texture, uint64_t overrideTextureID = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, Ref<Texture2D>& texture, uint64_t overrideTextureID = 0, const char* tooltip = nullptr);

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
		static void DrawField(Entity entity, const eastl::string& className, const eastl::string& fieldName);
		
		// Vec3 with reset button
		static void DrawVec3Control(const char* label, glm::vec3& values, const char* tooltip = nullptr, float resetValue = 0.0f);

		// Buttons
		static glm::vec2 GetIconButtonSize(const char* icon, const char* label);
		static bool IconButton(const char* icon, const char* label, ImVec4 iconColor = { 0.537f, 0.753f, 0.286f, 1.0f });
		static bool ToggleButton(const char* label, bool state, ImVec2 size = { 0, 0 }, float alpha = 1.0f, float pressedAlpha = 1.0f, ImGuiButtonFlags buttonFlags = ImGuiButtonFlags_None);

		static void ClippedText(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect, float wrap_width);
		static void ClippedText(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_display_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect, float wrap_width);

	private:
		static void PushID();
		static void PopID();

		static void BeginPropertyGrid(const char* label, const char* tooltip, bool rightAlignNextColumn = true);
		static void EndPropertyGrid();

	private:
		static int s_UIContextID;
		static uint32_t s_Counter;
		static char s_IDBuffer[16];
	};
}
