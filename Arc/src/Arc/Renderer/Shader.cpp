#include "arcpch.h"
#include "Arc/Renderer/Shader.h"

#include "Arc/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace ArcEngine
{
	Ref<Shader> Shader::Create(const eastl::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGLShader>(filepath);
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const eastl::string& name, const eastl::string& vertexSrc, const eastl::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ARC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		ARC_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void ShaderLibrary::Add(const eastl::string& name, const Ref<Shader>& shader)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		ARC_PROFILE_SCOPE();

		auto& name = shader->GetName();
		Add(name, shader);
	}

	Ref<Shader> ShaderLibrary::Load(const eastl::string& filepath)
	{
		ARC_PROFILE_SCOPE();

		auto shader = Shader::Create(filepath);
		Add(shader);
		m_ShaderPaths[shader->GetName()] = filepath;
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const eastl::string& name, const eastl::string& filepath)
	{
		ARC_PROFILE_SCOPE();

		auto shader = Shader::Create(filepath);
		Add(name, shader);
		m_ShaderPaths[name] = filepath;
		return shader;
	}

	void ShaderLibrary::ReloadAll()
	{
		ARC_PROFILE_SCOPE();

		eastl::string shaderName;
		for (auto [name, shader] : m_Shaders)
		{
			if (m_ShaderPaths.find_as(name) == m_ShaderPaths.end())
				continue;

			shader->Recompile(m_ShaderPaths.at(name));
		}
	}

	Ref<Shader> ShaderLibrary::Get(const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(Exists(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const eastl::string& name) const
	{
		ARC_PROFILE_SCOPE();

		return m_Shaders.find_as(name) != m_Shaders.end();
	}

}
