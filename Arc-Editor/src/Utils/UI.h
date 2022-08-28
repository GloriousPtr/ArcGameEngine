#pragma once

#include <ArcEngine.h>

#include <imgui/imgui.h>

namespace ArcEngine
{
	class UI
	{
	public:
		static void BeginProperties(ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame);
		static void EndProperties();

		// Strings
		static bool Property(const char* label, eastl::string& value, const char* tooltip = nullptr);
		static void Property(const char* label, const char* value, const char* tooltip = nullptr);

		// s/byte
		static bool Property(const char* label, int8_t& value, const char* tooltip = nullptr);
		static bool Property(const char* label, uint8_t& value, const char* tooltip = nullptr);
		static bool Property(const char* label, int8_t& value, int8_t min, int8_t max, const char* tooltip = nullptr);
		static bool Property(const char* label, uint8_t& value, uint8_t min, uint8_t max, const char* tooltip = nullptr);

		// u/short
		static bool Property(const char* label, int16_t& value, const char* tooltip = nullptr);
		static bool Property(const char* label, uint16_t& value, const char* tooltip = nullptr);
		static bool Property(const char* label, int16_t& value, int16_t min, int16_t max, const char* tooltip = nullptr);
		static bool Property(const char* label, uint16_t& value, uint16_t min, uint16_t max, const char* tooltip = nullptr);
		
		// u/int
		static bool Property(const char* label, int32_t& value, const char* tooltip = nullptr);
		static bool Property(const char* label, uint32_t& value, const char* tooltip = nullptr);
		static bool Property(const char* label, int32_t& value, int32_t min, int32_t max, const char* tooltip = nullptr);
		static bool Property(const char* label, uint32_t& value, uint32_t min, uint32_t max, const char* tooltip = nullptr);

		// u/long
		static bool Property(const char* label, int64_t& value, const char* tooltip = nullptr);
		static bool Property(const char* label, uint64_t& value, const char* tooltip = nullptr);
		static bool Property(const char* label, int64_t& value, int64_t min, int64_t max, const char* tooltip = nullptr);
		static bool Property(const char* label, uint64_t& value, uint64_t min, uint64_t max, const char* tooltip = nullptr);

		// Float
		static bool Property(const char* label, float& value, const char* tooltip = nullptr, float delta = 0.1f);
		static bool Property(const char* label, float& value, float min, float max, const char* tooltip = nullptr, const char* fmt = "%.3f");
		static bool Property(const char* label, float& value, float delta, float min, float max, const char* tooltip = nullptr, const char* fmt = "%.3f");

		// Double
		static bool Property(const char* label, double& value, const char* tooltip = nullptr, float delta = 0.1f);
		static bool Property(const char* label, double& value, float min, float max, const char* tooltip = nullptr, const char* fmt = "%.6f");
		static bool Property(const char* label, double& value, float delta, float min, float max, const char* tooltip = nullptr, const char* fmt = "%.6f");

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
		static bool Property(const char* label, Ref<TextureCubemap>& texture, uint32_t overrideTextureID = 0, const char* tooltip = nullptr);
		static bool Property(const char* label, Ref<Texture2D>& texture, uint32_t overrideTextureID = 0, const char* tooltip = nullptr);

		// Field
		static void DrawField(Field& field);
		
		// Vec3 with reset button
		static void DrawVec3Control(const char* label, glm::vec3& values, const char* tooltip = nullptr, float resetValue = 0.0f, float columnWidth = 100.0f);

		// Buttons
		static bool IconButton(const char* icon, const char* label, ImVec4 iconColor = { 0.537f, 0.753f, 0.286f, 1.0f });
		static bool ToggleButton(const char* label, bool state, ImVec2 size = { 0, 0 }, float alpha = 1.0f, float pressedAlpha = 1.0f, ImGuiButtonFlags buttonFlags = ImGuiButtonFlags_None);

	private:
		static void PushID();
		static void PopID();

		static void BeginPropertyGrid(const char* label, const char* tooltip, bool rightAlignNextColumn = true);
		static void EndPropertyGrid();
	};
}
