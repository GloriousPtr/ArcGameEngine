#pragma once

#include <functional>

namespace ArcEngine
{
	class ProjectBuilder
	{
	public:
		[[nodiscard]] static bool GenerateProjectFiles();
		static void BuildProject(bool async, const std::function<void(bool)>& onComplete = nullptr);
	};
}
