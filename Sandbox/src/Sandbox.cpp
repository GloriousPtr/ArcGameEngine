#include <Arc.h>

class Sandbox : public Arc::Application
{
public:
	Sandbox()
	{
		
	}

	~Sandbox()
	{
		
	}
};

Arc::Application* Arc::CreateApplication()
{
	return new Sandbox();
}
