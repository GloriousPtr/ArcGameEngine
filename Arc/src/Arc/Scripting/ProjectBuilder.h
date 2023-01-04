#pragma once

#include <functional>

namespace ArcEngine
{
	class ProjectBuilder
	{
	public:
		[[nodiscard]] static bool GenerateProjectFiles();
		[[nodiscard]] static bool BuildProject(const std::function<void()>& onComplete = nullptr);
	};
}
