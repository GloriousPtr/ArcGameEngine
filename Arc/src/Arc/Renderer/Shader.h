#pragma once

#include "Arc/Renderer/Buffer.h"
#include "Arc/Utils/StringUtils.h"
#include "glm/gtc/type_ptr.hpp"

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

		[[nodiscard]] virtual MaterialPropertyMap& GetMaterialProperties() = 0;
		[[nodiscard]] virtual const std::string& GetName() const = 0;

		template<typename T>
		void SetData(const std::string& name, const T& data, uint32_t size = 0, uint32_t offset = 0)
		{
			SetDataImpl(name, &data, (size == 0 ? sizeof(T) : size), offset);
		}
		
	private:
		virtual void SetDataImpl(const std::string& name, const void* data, uint32_t size, uint32_t offset) = 0;

	public:
		[[nodiscard]] static Ref<Shader> Create(const std::filesystem::path& filepath);
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
