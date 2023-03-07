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
			m_EditorLayer = new EditorLayer();
			PushLayer(m_EditorLayer);
		}

		~ArcEditor() override
		{
			PopLayer(m_EditorLayer);
			delete m_EditorLayer;
		}

	private:
		EditorLayer* m_EditorLayer;
	};

	Application* CreateApplication()
	{
		return new ArcEditor();
	}
}
