#pragma once
#include "Arc/Core/Base.h"
#include "Arc/Core/Application.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

extern ArcEngine::Application* ArcEngine::CreateApplication(ApplicationCommandLineArgs args);

#if defined(ARC_PLATFORM_WINDOWS) && defined(ARC_DIST)
int WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] char*, [[maybe_unused]] int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
#if defined(ARC_PLATFORM_WINDOWS) && defined(ARC_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

#if defined(ARC_PLATFORM_WINDOWS) && defined(ARC_DIST)
	int argsNum = __argc;
	char** args = __argv;
#else
	int argsNum = argc;
	char** args = argv;
#endif

	ArcEngine::Log::Init();
	auto* app = ArcEngine::CreateApplication({ argsNum, args });
	app->Run();
	delete app;
}
