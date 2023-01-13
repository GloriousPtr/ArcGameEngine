#pragma once

namespace ArcEngine
{
	class Texture2D;
	class Shader;

	class Material
	{
		using MaterialData = void*;

	public:
		explicit Material(const std::filesystem::path& shaderPath = "assets/shaders/PBR.glsl");
		virtual ~Material();

		Material(const Material& other) = default;
		Material(Material&& other) = default;

		void Invalidate();
		void Bind() const;
		void Unbind() const;
		[[nodiscard]] Ref<Shader> GetShader() const;
		[[nodiscard]] Ref<Texture2D> GetTexture(uint32_t slot);
		void SetTexture(uint32_t slot, const Ref<Texture2D>& texture);

		template<typename T>
		[[nodiscard]] T GetData(const std::string& name) const
		{
			MaterialData value = GetData_Internal(name);
			return *static_cast<T*>(value);
		}

		template<typename T>
		void SetData(const std::string& name, T data) const
		{
			SetData_Internal(name, &data);
		}

	private:
		[[nodiscard]] MaterialData GetData_Internal(const std::string& name) const;
		void SetData_Internal(const std::string& name, MaterialData data) const;

	private:
		Ref<Shader> m_Shader = nullptr;
		char* m_Buffer = nullptr;
		size_t m_BufferSizeInBytes = 0;
		std::unordered_map<uint32_t, Ref<Texture2D>> m_Textures;

		static Ref<Texture2D> s_WhiteTexture;
	};
}
