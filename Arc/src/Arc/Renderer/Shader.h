#pragma once

#include "Arc/Renderer/Buffer.h"
#include "Arc/Utils/StringUtils.h"

namespace ArcEngine
{
	enum class MaterialPropertyType
	{
		None = 0,
		Texture2D,
		Texture2DBindless,
		Bool,
		Int,
		UInt,
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
		uint32_t BindingOffset;

		std::string DisplayName;
		bool IsSlider;
		bool IsColor;

		int32_t Slot = -1;

		[[nodiscard]] bool IsValid() const
		{
			return Slot != -1;
		}
	};

	using MaterialPropertyMap = std::unordered_map<std::string, MaterialProperty, UM_StringTransparentEquality>;

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Recompile(const std::filesystem::path& path) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value, uint32_t offset = 0) = 0;
		virtual void SetUInt(const std::string& name, unsigned int value, uint32_t offset = 0) = 0;
		virtual void SetIntArray(const std::string& name, const int* values, uint32_t count, uint32_t offset = 0) = 0;
		virtual void SetFloat(const std::string& name, float value, uint32_t offset = 0) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value, uint32_t offset = 0) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value, uint32_t offset = 0) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value, uint32_t offset = 0) = 0;
		virtual void SetMat3(const std::string& name, const glm::mat3& value, uint32_t offset = 0) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value, uint32_t offset = 0) = 0;

		[[nodiscard]] virtual MaterialPropertyMap& GetMaterialProperties() = 0;

		[[nodiscard]] virtual const std::string& GetName() const = 0;

		[[nodiscard]] static Ref<Shader> Create(const std::filesystem::path& filepath);

		[[nodiscard]] static constexpr size_t GetSizeInBytes(MaterialPropertyType type)
		{
			switch (type)
			{
				case MaterialPropertyType::None:					return 0;
				case MaterialPropertyType::Texture2D:				return sizeof(int32_t);
				case MaterialPropertyType::Texture2DBindless:		return sizeof(uint32_t);
				case MaterialPropertyType::Bool:					[[fallthrough]];
				case MaterialPropertyType::Int:						return sizeof(int32_t);
				case MaterialPropertyType::UInt:					return sizeof(uint32_t);
				case MaterialPropertyType::Float:					return sizeof(float);
				case MaterialPropertyType::Float2:					return sizeof(glm::vec2);
				case MaterialPropertyType::Float3:					return sizeof(glm::vec3);
				case MaterialPropertyType::Float4:					return sizeof(glm::vec4);
			}

			return 0;
		}
	};

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::filesystem::path& filepath);
		void ReloadAll();

		[[nodiscard]] Ref<Shader> Get(const std::string& name);

		[[nodiscard]] bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>, UM_StringTransparentEquality> m_Shaders;
		std::unordered_map<std::string, std::string, UM_StringTransparentEquality> m_ShaderPaths;
	};
}
