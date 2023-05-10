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

		[[nodiscard]] virtual const std::string& GetName() const = 0;

		[[nodiscard]] static Ref<Shader> Create(const std::filesystem::path& filepath, ShaderType type);
	};

	class PipelineLibrary
	{
	public:
		Ref<PipelineState> Load(const std::filesystem::path& shaderPath, const PipelineSpecification& spec);
		void ReloadAll();

		[[nodiscard]] Ref<PipelineState> Get(const std::string& name);

		[[nodiscard]] bool Exists(const std::string& name) const;
	private:

		str_umap<Ref<PipelineState>> m_Pipelines;
		str_umap<Ref<Shader>> m_Shaders;
		str_umap<std::string> m_ShaderPaths;
	};
}
