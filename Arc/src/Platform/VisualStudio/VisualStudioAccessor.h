#pragma once

namespace ArcEngine
{
	class VisualStudioAccessor
	{
	public:
		static void RunVisualStudio();
		static void OpenFile(const eastl::string& filepath);
	};
}
