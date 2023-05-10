#pragma once

#include "Arc/Utils/StringUtils.h"

namespace ArcEngine
{
	struct PipelineSpecification;
	class PipelineState;

	enum class ShaderType
	{
		None = 0,
		Vertex,
		Pixel,
		Compute,

		Fragment = Pixel
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Recompile(const std::filesystem::path& path) = 0;

		[[nodiscard]] virtual const eastl::string& GetName() const = 0;

		[[nodiscard]] static Ref<Shader> Create(const std::filesystem::path& filepath, ShaderType type);
	};

	class PipelineLibrary
	{
	public:
		Ref<PipelineState> Load(const std::filesystem::path& shaderPath, const PipelineSpecification& spec);
		void ReloadAll();

		[[nodiscard]] Ref<PipelineState> Get(const eastl::string& name);

		[[nodiscard]] bool Exists(const eastl::string& name) const;
	private:

		eastl::hash_map<eastl::string, Ref<PipelineState>> m_Pipelines;
		eastl::hash_map<eastl::string, Ref<Shader>> m_Shaders;
		eastl::hash_map<eastl::string, eastl::string> m_ShaderPaths;
	};
}
