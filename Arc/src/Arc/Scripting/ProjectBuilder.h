#pragma once

#include <functional>

namespace ArcEngine
{
	class ProjectBuilder
	{
	public:
		static bool GenerateProjectFiles();
		static bool BuildProject();
	};
}
