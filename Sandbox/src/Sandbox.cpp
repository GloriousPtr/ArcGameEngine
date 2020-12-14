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
		if(ArcEngine::Input::IsKeyPressed(ARC_KEY_TAB))
			ARC_TRACE("Tab key is pressed (poll)!");
	}

	void OnEvent(ArcEngine::Event& event) override
	{
		if(event.GetEventType() == ArcEngine::EventType::KeyPressed)
		{
			ArcEngine::KeyPressedEvent& e = (ArcEngine::KeyPressedEvent&)event;
			if(e.GetKeyCode() == ARC_KEY_TAB)
				ARC_TRACE("Tab key is pressed (event)!");
			ARC_TRACE("{0}", (char)e.GetKeyCode());
		}
	}
};

class Sandbox : public ArcEngine::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new ArcEngine::ImGuiLayer());
	}

	~Sandbox()
	{
		
	}
};

ArcEngine::Application* ArcEngine::CreateApplication()
{
	return new Sandbox();
}
