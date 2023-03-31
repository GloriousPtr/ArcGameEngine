#pragma once
#include "Arc/Core/Base.h"
#include "Arc/Core/Application.h"

extern ArcEngine::Application* ArcEngine::CreateApplication(ApplicationCommandLineArgs args);

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	ArcEngine::Log::Init();
	auto* app = ArcEngine::CreateApplication({ argc, argv });
	app->Run();
	delete app;
}
