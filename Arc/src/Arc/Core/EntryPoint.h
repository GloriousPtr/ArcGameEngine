#pragma once
#include "Arc/Core/Core.h"

#ifdef ARC_PLATFORM_WINDOWS

extern ArcEngine::Application* ArcEngine::CreateApplication();

int main(int argc, char** argv)
{
	ArcEngine::Log::Init();
	
	ARC_PROFILE_BEGIN_SESSION("Startup", "ArcEngineProfile-Startup.json");
	auto* app = ArcEngine::CreateApplication();
	ARC_PROFILE_END_SESSION();
	
	ARC_PROFILE_BEGIN_SESSION("Runtime", "ArcEngineProfile-Runtime.json");
	app->Run();
	ARC_PROFILE_END_SESSION();
	
	ARC_PROFILE_BEGIN_SESSION("Shutdown", "ArcEngineProfile-Shutdown.json");
	delete app;
	ARC_PROFILE_END_SESSION();
}

#endif
