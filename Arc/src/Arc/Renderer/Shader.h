#pragma once

#include "Arc/Renderer/Buffer.h"
#include "Arc/Utils/StringUtils.h"

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

	struct MaterialProperty
	{
		MaterialPropertyType Type;
		size_t SizeInBytes;
		size_t OffsetInBytes;

		std::string DisplayName;
		bool IsSlider;
		bool IsColor;
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Recompile(const std::filesystem::path& path) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, const int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat3(const std::string& name, const glm::mat3& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		
		[[nodiscard]] virtual std::unordered_map<std::string, MaterialProperty, UM_StringTransparentEquality>& GetMaterialProperties() = 0;

		[[nodiscard]] virtual const std::string& GetName() const = 0;

		[[nodiscard]] static Ref<Shader> Create(const std::filesystem::path& filepath, const BufferLayout& layout = {});

		[[nodiscard]] static constexpr size_t GetSizeInBytes(MaterialPropertyType type)
		{
			switch (type)
			{
				case MaterialPropertyType::None: return 0;
				case MaterialPropertyType::Sampler2D: [[fallthrough]];
				case MaterialPropertyType::Bool: [[fallthrough]];
				case MaterialPropertyType::Int: return sizeof(int32_t);
				case MaterialPropertyType::Float: return sizeof(float);
				case MaterialPropertyType::Float2: return sizeof(glm::vec2);
				case MaterialPropertyType::Float3: return sizeof(glm::vec3);
				case MaterialPropertyType::Float4: return sizeof(glm::vec4);
			}

			return 0;
		}
	};

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		[[nodiscard]] Ref<Shader> Load(const std::filesystem::path& filepath);
		void ReloadAll();

		[[nodiscard]] Ref<Shader> Get(const std::string& name);

		[[nodiscard]] bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>, UM_StringTransparentEquality> m_Shaders;
		std::unordered_map<std::string, std::string, UM_StringTransparentEquality> m_ShaderPaths;
	};
}
