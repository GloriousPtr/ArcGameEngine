#include <ArcEngine.h>
#include <Arc/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace ArcEngine
{
	class ArcEditor : public Application
	{
	public:
		ArcEditor()
			: Application("Arc Editor")
		{
			PushLayer(new EditorLayer());
		}

		~ArcEditor() override = default;
	};

	Application* CreateApplication()
	{
		return new ArcEditor();
	}
}
