#pragma once

#include <string>

namespace ArcEngine
{
	class VisualStudioAccessor
	{
	public:
		static void RunVisualStudio();
		static void OpenFile(const std::string& filepath, uint32_t goToLine = 0, bool selectLine = false);
		static bool AddFile(const std::string& filepath, bool open = true);
		static bool GenerateProjectFiles();
		static bool BuildSolution(const std::function<void()>& onComplete = nullptr);
	};
}
