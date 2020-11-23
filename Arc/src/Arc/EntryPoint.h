#pragma once

#ifdef ARC_PLATFORM_WINDOWS

extern Arc::Application* Arc::CreateApplication();

int main(int argc, char** argv)
{
	auto* app = Arc::CreateApplication();
	app->Run();
	delete app;
}

#endif
