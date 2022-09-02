#pragma once

#include "Arc/Core/Base.h"
#include "Shader.h"
#include "Texture.h"

namespace ArcEngine
{
	class Material
	{
		using MaterialData = void*;

	public:
		explicit Material(const char* shaderPath = "assets/shaders/PBR.glsl");
		virtual ~Material();

		Material(const Material& other) = default;
		Material(Material&& other) = default;

		void Invalidate();
		void Bind();
		void Unbind() const;
		Ref<Shader> GetShader() const { return m_Shader; }
		Ref<Texture2D> GetTexture(uint32_t slot) const { return m_Textures.at(slot); }
		void SetTexture(uint32_t slot, Ref<Texture2D> texture) { m_Textures[slot] = texture; }

		template<typename T>
		T GetData(const char* name)
		{
			MaterialData value = GetData_Internal(name);
			return *(T*)(value);
		}

		template<typename T>
		void SetData(const char* name, T data)
		{
			SetData_Internal(name, &data);
		}

	private:
		MaterialData GetData_Internal(const char* name);
		void SetData_Internal(const char* name, MaterialData data);

	private:
		Ref<Shader> m_Shader = nullptr;
		char* m_Buffer = nullptr;
		size_t m_BufferSizeInBytes = 0;
		eastl::hash_map<uint32_t, Ref<Texture2D>> m_Textures;

		static Ref<Texture2D> s_WhiteTexture;
	};
}
