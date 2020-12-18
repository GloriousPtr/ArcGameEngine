#include <ArcEngine.h>

#include "imgui/imgui.h"

class ExampleLayer : public ArcEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	virtual void OnUpdate() override
	{
		if(ArcEngine::Input::IsKeyPressed(ARC_KEY_TAB))
			ARC_TRACE("Tab key is pressed (poll)!");
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();
	}
	
	virtual void OnEvent(ArcEngine::Event& event) override
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
	}

	~Sandbox()
	{
		
	}
};

ArcEngine::Application* ArcEngine::CreateApplication()
{
	return new Sandbox();
}
