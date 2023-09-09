#pragma once

#include <functional>

namespace ArcEngine
{
	class ProjectBuilder
	{
	public:
		static void GenerateProjectFiles(const std::function<void(bool)>& onComplete);
		static void BuildProject(bool async, const std::function<void(bool)>& onComplete = nullptr);
	};
}
