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
			m_EditorLayer = CreateScope<EditorLayer>();
			PushLayer(m_EditorLayer.get());
		}

		~ArcEditor() override
		{
			ARC_CORE_ASSERT(m_EditorLayer, "Editor Layer doesn't exist!");
			PopLayer(m_EditorLayer.get());
		}

	private:
		Scope<EditorLayer> m_EditorLayer;
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
