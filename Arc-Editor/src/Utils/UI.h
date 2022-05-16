#pragma once

#include <ArcEngine.h>

namespace ArcEngine
{
	class UI
	{
	public:
		static void BeginProperties();
		static void EndProperties();

		static bool Property(const char* label, std::string& value);
		static void Property(const char* label, const char* value);
		static bool Property(const char* label, int32_t& value);
		static bool Property(const char* label, uint32_t& value);
		static bool Property(const char* label, int32_t& value, int32_t min, int32_t max);
		static bool Property(const char* label, uint32_t& value, uint32_t min, uint32_t max);
		static bool Property(const char* label, float& value, float delta = 0.1f);
		static bool Property(const char* label, float& value, float min, float max, const char* fmt = "%.3f");
		static bool Property(const char* label, float& value, float delta, float min, float max, const char* fmt = "%.3f");
		static bool Property(const char* label, glm::vec2& value, float delta = 0.1f);
		static bool Property(const char* label, glm::vec3& value, float delta = 0.1f);
		static bool Property(const char* label, glm::vec4& value, float delta = 0.1f);
		static bool Property(const char* label, bool& flag);

		static bool Property(const char* label, int& value, const char** dropdownStrings, size_t count);

		static bool PropertyColor3(const char* label, glm::vec3& color);
		static bool PropertyColor4(const char* label, glm::vec4& color);
		static bool PropertyColor4as3(const char* label, glm::vec4& color);

		static bool Property(const char* label, Ref<TextureCubemap>& texture, uint32_t overrideTextureID = 0);
		static bool Property(const char* label, Ref<Texture2D>& texture, uint32_t overrideTextureID = 0);

		static void DrawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

		static void DrawRowsBackground(int row_count, float line_height, float x1, float x2, float y_offset, uint32_t col_even, uint32_t col_odd);

	private:
		static void PushID();
		static void PopID();

		static void BeginPropertyGrid(const char* label, float rowHeight = 0.0f, bool rightAlignNextColumn = true);
		static void EndPropertyGrid();
	};
}
