#pragma once

#ifdef ARC_PLATFORM_WINDOWS

extern ArcEngine::Application* ArcEngine::CreateApplication();

int main(int argc, char** argv)
{
	ArcEngine::Log::Init();
	ARC_CORE_TRACE("Initialized Log!");
	
	auto* app = ArcEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif
