#include <ArcEngine.h>

class Sandbox : public ArcEngine::Application
{
public:
	Sandbox()
	{
		
	}

	~Sandbox()
	{
		
	}
};

ArcEngine::Application* ArcEngine::CreateApplication()
{
	return new Sandbox();
}
