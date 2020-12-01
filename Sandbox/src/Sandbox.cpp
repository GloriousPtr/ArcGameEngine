#include <ArcEngine.h>

class ExampleLayer : public ArcEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		ARC_INFO("ExampleLayer::Update");
	}

	void OnEvent(ArcEngine::Event& event) override
	{
		ARC_TRACE("{0}", event);
	}
};

class Sandbox : public ArcEngine::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{
		
	}
};

ArcEngine::Application* ArcEngine::CreateApplication()
{
	return new Sandbox();
}
