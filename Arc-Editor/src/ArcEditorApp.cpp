#include <ArcEngine.h>
#include <Arc/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace ArcEngine
{
	class ArcEditor : public Application
	{
	public:
		ArcEditor(const ApplicationSpecification& spec)
			: Application(spec)
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
		EditorLayer* m_EditorLayer = nullptr;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		// Set working directory to currently running program
		if (args.Count > 0)
		{
			const std::filesystem::path enginePath(args[0]);
			std::filesystem::current_path(enginePath.parent_path());
		}

		const ApplicationSpecification spec
		{
			.Name = "Arc Editor",
			.CommandLineArgs = args
		};

		return new ArcEditor(spec);
	}
}
