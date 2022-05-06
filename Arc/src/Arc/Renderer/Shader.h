#pragma once

#include <string>
#include <unordered_map>

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

		virtual void Recompile(const std::string& path) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat3(const std::string& name, const glm::mat3& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetUniformBlock(const std::string& name, uint32_t blockIndex) = 0;
		
		virtual std::map<std::string, MaterialProperty>& GetMaterialProperties() = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(const std::string& filepath);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);
		void ReloadAll();

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
		std::unordered_map<std::string, std::string> m_ShaderPaths;
	};
}

