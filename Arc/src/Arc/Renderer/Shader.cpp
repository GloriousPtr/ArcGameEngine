#include "arcpch.h"
#include "Arc/Renderer/Shader.h"

#include "PipelineState.h"
#include "Arc/Renderer/Renderer.h"
#include "Platform/Dx12/Dx12Shader.h"

namespace ArcEngine
{
	Ref<Shader> Shader::Create(const std::filesystem::path& filepath)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!") return nullptr;
			case RendererAPI::API::Dx12:	return CreateRef<Dx12Shader>(filepath);
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}

	Ref<PipelineState> PipelineLibrary::Load(const std::filesystem::path& shaderPath, const PipelineSpecification& spec)
	{
		ARC_PROFILE_SCOPE()

		const auto& name = shaderPath.filename().string();
		ARC_CORE_ASSERT(!Exists(name), "Shader already exists!")

		const auto shader = Shader::Create(shaderPath);
		if (!shader)
			return nullptr;

		auto pipeline = PipelineState::Create(shader, spec);
		m_Pipelines[name] = pipeline;
		m_ShaderPaths[shader->GetName()] = shaderPath.string();

		return pipeline;
	}

	void PipelineLibrary::ReloadAll()
	{
		ARC_PROFILE_SCOPE()

		std::string shaderName;
		for (const auto& [name, pipeline] : m_Pipelines)
		{
			const auto& it = m_ShaderPaths.find(name);
			if (it == m_ShaderPaths.end())
				continue;

			//pipeline->Recompile(it->second);
		}
	}

	Ref<PipelineState> PipelineLibrary::Get(const std::string& name)
	{
		ARC_PROFILE_SCOPE()

		ARC_CORE_ASSERT(Exists(name), "Pipeline not found!")
		return m_Pipelines[name];
	}

	bool PipelineLibrary::Exists(const std::string& name) const
	{
		ARC_PROFILE_SCOPE()

		return m_Pipelines.contains(name);
	}
}
