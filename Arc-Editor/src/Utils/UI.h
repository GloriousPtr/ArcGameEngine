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
		static bool Property(const char* label, eastl::string& value);
		static void Property(const char* label, const char* value);

		// s/byte
		static bool Property(const char* label, int8_t& value);
		static bool Property(const char* label, uint8_t& value);
		static bool Property(const char* label, int8_t& value, int8_t min, int8_t max);
		static bool Property(const char* label, uint8_t& value, uint8_t min, uint8_t max);

		// u/short
		static bool Property(const char* label, int16_t& value);
		static bool Property(const char* label, uint16_t& value);
		static bool Property(const char* label, int16_t& value, int16_t min, int16_t max);
		static bool Property(const char* label, uint16_t& value, uint16_t min, uint16_t max);
		
		// u/int
		static bool Property(const char* label, int32_t& value);
		static bool Property(const char* label, uint32_t& value);
		static bool Property(const char* label, int32_t& value, int32_t min, int32_t max);
		static bool Property(const char* label, uint32_t& value, uint32_t min, uint32_t max);

		// u/long
		static bool Property(const char* label, int64_t& value);
		static bool Property(const char* label, uint64_t& value);
		static bool Property(const char* label, int64_t& value, int64_t min, int64_t max);
		static bool Property(const char* label, uint64_t& value, uint64_t min, uint64_t max);

		// Float
		static bool Property(const char* label, float& value, float delta = 0.1f);
		static bool Property(const char* label, float& value, float min, float max, const char* fmt = "%.3f");
		static bool Property(const char* label, float& value, float delta, float min, float max, const char* fmt = "%.3f");

		// Double
		static bool Property(const char* label, double& value, double delta = 0.1f);
		static bool Property(const char* label, double& value, double min, double max, const char* fmt = "%.6f");
		static bool Property(const char* label, double& value, double delta, double min, double max, const char* fmt = "%.6f");

		// Vec2/3/4
		static bool Property(const char* label, glm::vec2& value, float delta = 0.1f);
		static bool Property(const char* label, glm::vec3& value, float delta = 0.1f);
		static bool Property(const char* label, glm::vec4& value, float delta = 0.1f);

		// Bool
		static bool Property(const char* label, bool& flag);

		// Dropdown
		static bool Property(const char* label, int& value, const char** dropdownStrings, size_t count);

		// Colors
		static bool PropertyColor3(const char* label, glm::vec3& color);
		static bool PropertyColor4(const char* label, glm::vec4& color);
		static bool PropertyColor4as3(const char* label, glm::vec4& color);

		// 2D/3D Textures
		static bool Property(const char* label, Ref<TextureCubemap>& texture, uint32_t overrideTextureID = 0);
		static bool Property(const char* label, Ref<Texture2D>& texture, uint32_t overrideTextureID = 0);

		// Vec3 with reset button
		static void DrawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

		// Buttons
		static bool IconButton(const char* icon, const char* label, ImVec4 iconColor = { 0.537f, 0.753f, 0.286f, 1.0f });
		static bool UI::ToggleButton(const char* label, bool state, ImVec2 size = { 0, 0 }, float alpha = 1.0f, float pressedAlpha = 1.0f, ImGuiButtonFlags buttonFlags = ImGuiButtonFlags_None);

	private:
		static void PushID();
		static void PopID();

		static void BeginPropertyGrid(const char* label, bool rightAlignNextColumn = true);
		static void EndPropertyGrid();
	};
}
