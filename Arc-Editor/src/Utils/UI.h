#pragma once

#include <ArcEngine.h>

namespace ArcEngine
{
	class UI
	{
	public:
		static void PushID();
		static void PopID();

		static void BeginPropertyGrid();
		static void Property(const char* label);
		static bool Property(const char* label, std::string& value);
		static void Property(const char* label, const char* value);
		static bool Property(const char* label, int& value);
		static bool Property(const char* label, int& value, int min, int max);
		static bool Property(const char* label, float& value, float delta = 0.1f);
		static bool Property(const char* label, float& value, float min, float max, const char* fmt = "%.3f");
		static bool Property(const char* label, glm::vec2& value, float delta = 0.1f);
		static bool Property(const char* label, bool& flag);
		static void EndPropertyGrid();
	};
}

