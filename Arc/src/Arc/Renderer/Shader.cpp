#include "arcpch.h"
#include "Arc/Renderer/Shader.h"

#include "PipelineState.h"
#include "Arc/Renderer/Renderer.h"
#include "Platform/Dx12/Dx12Shader.h"

namespace ArcEngine
{
	Ref<Shader> Shader::Create(const std::filesystem::path& filepath, ShaderType type)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::Dx12:	return CreateRef<Dx12Shader>(filepath, type);
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<PipelineState> PipelineLibrary::Load(const std::filesystem::path& shaderPath, const PipelineSpecification& spec)
	{
		ARC_PROFILE_SCOPE();

		std::string nameStr = shaderPath.filename().string();
		eastl::string name = nameStr.c_str();
		ARC_CORE_ASSERT(!Exists(name.c_str()), "Shader already exists!");

		const auto shader = Shader::Create(shaderPath, spec.Type);
		if (!shader)
			return nullptr;

		auto pipeline = PipelineState::Create(shader, spec);
		m_Pipelines[name] = pipeline;
		m_Shaders[name] = shader;

		auto path = shaderPath.string();
		m_ShaderPaths[shader->GetName()] = path.c_str();

		return pipeline;
	}

	void PipelineLibrary::ReloadAll()
	{
		ARC_PROFILE_SCOPE();

		eastl::string shaderName;
		for (const auto& [name, pipeline] : m_Pipelines)
		{
			const auto& it = m_ShaderPaths.find(name);
			if (it == m_ShaderPaths.end())
				continue;

			//pipeline->Recompile(it->second);
		}
	}

	Ref<PipelineState> PipelineLibrary::Get(const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(Exists(name), "Pipeline not found!");
		return m_Pipelines[name];
	}

	bool PipelineLibrary::Exists(const eastl::string& name) const
	{
		ARC_PROFILE_SCOPE();

		return m_Pipelines.find(name) != m_Pipelines.end();
	}
}
