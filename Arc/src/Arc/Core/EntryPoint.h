#pragma once
#include "Arc/Core/Base.h"

#ifdef ARC_PLATFORM_WINDOWS

extern ArcEngine::Application* ArcEngine::CreateApplication();

int main(int argc, char** argv)
{
	ArcEngine::Log::Init();
	auto* app = ArcEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif
