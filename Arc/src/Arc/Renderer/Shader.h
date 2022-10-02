#pragma once

#include <EASTL/hash_map.h>

#include <EASTL/string.h>
#include <glm/glm.hpp>

namespace ArcEngine
{
	enum class MaterialPropertyType
	{
		None = 0,
		Sampler2D,
		Bool,
		Int,
		Float,
		Float2,
		Float3,
		Float4,
	};

	static size_t GetSizeInBytes(MaterialPropertyType type)
	{
		switch (type)
		{
			case MaterialPropertyType::None					: return 0;
			case MaterialPropertyType::Sampler2D			: return sizeof(int32_t);
			case MaterialPropertyType::Bool					: return sizeof(int32_t);
			case MaterialPropertyType::Int					: return sizeof(int32_t);
			case MaterialPropertyType::Float				: return sizeof(float);
			case MaterialPropertyType::Float2				: return sizeof(glm::vec2);
			case MaterialPropertyType::Float3				: return sizeof(glm::vec3);
			case MaterialPropertyType::Float4				: return sizeof(glm::vec4);
		}

		return 0;
	}

	struct MaterialProperty
	{
		MaterialPropertyType Type;
		size_t SizeInBytes;
		size_t OffsetInBytes;
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Recompile(const eastl::string& path) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const eastl::string& name, int value) = 0;
		virtual void SetIntArray(const eastl::string& name, const int* values, uint32_t count) = 0;
		virtual void SetFloat(const eastl::string& name, float value) = 0;
		virtual void SetFloat2(const eastl::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const eastl::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const eastl::string& name, const glm::vec4& value) = 0;
		virtual void SetMat3(const eastl::string& name, const glm::mat3& value) = 0;
		virtual void SetMat4(const eastl::string& name, const glm::mat4& value) = 0;
		virtual void SetUniformBlock(const eastl::string& name, uint32_t blockIndex) = 0;
		
		virtual eastl::hash_map<eastl::string, MaterialProperty>& GetMaterialProperties() = 0;

		virtual const eastl::string& GetName() const = 0;

		static Ref<Shader> Create(const eastl::string& filepath);
		static Ref<Shader> Create(const eastl::string& name, const eastl::string& vertexSrc, const eastl::string& fragmentSrc);
	};

	class ShaderLibrary
	{
	public:
		void Add(const eastl::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const eastl::string& filepath);
		Ref<Shader> Load(const eastl::string& name, const eastl::string& filepath);
		void ReloadAll();

		Ref<Shader> Get(const eastl::string& name);

		bool Exists(const eastl::string& name) const;
	private:
		eastl::hash_map<eastl::string, Ref<Shader>> m_Shaders;
		eastl::hash_map<eastl::string, eastl::string> m_ShaderPaths;
	};
}
