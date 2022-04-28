#pragma once

#include <ArcEngine.h>

namespace ArcEngine
{
	class UI
	{
		static void PushID();
		static void PopID();

		static void BeginPropertyGrid();
		static void EndPropertyGrid();
	public:
		static void Property(const char* label);
		static bool Property(const char* label, std::string& value);
		static void Property(const char* label, const char* value);
		static bool Property(const char* label, int& value);
		static bool Property(const char* label, int& value, int min, int max);
		static bool Property(const char* label, float& value, float delta = 0.1f);
		static bool Property(const char* label, float& value, float min, float max, const char* fmt = "%.3f");
		static bool Property(const char* label, float& value, float delta, float min, float max, const char* fmt = "%.3f");
		static bool Property(const char* label, glm::vec2& value, float delta = 0.1f);
		static bool Property(const char* label, bool& flag);

		static bool Property(const char* label, int& value, const char** dropdownStrings, size_t count);

		static bool PropertyColor3(const char* label, glm::vec3& color);
		static bool PropertyColor4(const char* label, glm::vec4& color);
		static bool PropertyColor4as3(const char* label, glm::vec4& color);

		static void DrawRowsBackground(int row_count, float line_height, float x1, float x2, float y_offset, uint32_t col_even, uint32_t col_odd);
	};
}

