#pragma once
#include "Arc/Core/Base.h"

extern ArcEngine::Application* ArcEngine::CreateApplication();

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	ArcEngine::Log::Init();
	auto* app = ArcEngine::CreateApplication();
	app->Run();
	delete app;
}
