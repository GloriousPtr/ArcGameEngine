#pragma once

#include "Core.h"

namespace ArcEngine
{
	class ARC_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// Should be defined in CLIENT
	Application* CreateApplication();
}
