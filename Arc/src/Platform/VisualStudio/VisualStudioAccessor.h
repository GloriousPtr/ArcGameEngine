#pragma once

#include <EASTL/string.h>

namespace ArcEngine
{
	class VisualStudioAccessor
	{
	public:
		static void RunVisualStudio();
		static void OpenFile(const eastl::string& filepath, uint32_t goToLine = 0, bool selectLine = false);
	};
}
