#pragma once
#include "Arc/Core/Base.h"
#include "Arc/Core/Application.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

extern ArcEngine::Application* ArcEngine::CreateApplication(ApplicationCommandLineArgs args);

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);

	ArcEngine::Log::Init();
	auto* app = ArcEngine::CreateApplication({ argc, argv });
	app->Run();
	delete app;
}
