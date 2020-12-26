#include <ArcEngine.h>
#include <Arc/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "ExampleLayer.h"

class Sandbox : public ArcEngine::Application
{
public:
	Sandbox()
	{
		// PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{
	}
};

ArcEngine::Application* ArcEngine::CreateApplication()
{
	return new Sandbox();
}
