#pragma once
#include "Arc/Core/Base.h"

extern ArcEngine::Application* ArcEngine::CreateApplication();

int main(int argc, char** argv)
{
	ArcEngine::Log::Init();
	auto* app = ArcEngine::CreateApplication();
	app->Run();
	delete app;
}
