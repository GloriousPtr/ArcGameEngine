#pragma once

#include "Core.h"

namespace Arc
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
