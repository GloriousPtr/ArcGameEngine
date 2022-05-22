#pragma once

#include "Arc/Core/Base.h"
#include "Shader.h"
#include "Texture.h"

namespace ArcEngine
{
	class Material
	{
	public:
		Material(const char* shaderPath = "assets/shaders/PBR.glsl");
		virtual ~Material();

		void Invalidate();
		void Bind();
		void Unbind();
		Ref<Shader> GetShader() { return m_Shader; }
		Ref<Texture2D> GetTexture(uint32_t slot) { return m_Textures.at(slot); }
		void SetTexture(uint32_t slot, Ref<Texture2D> texture) { m_Textures[slot] = texture; }

		template<typename T>
		T GetData(const char* name)
		{
			void* value = GetData_Internal(name);
			return *reinterpret_cast<T*>(value);
		}

		template<typename T>
		void SetData(const char* name, T data)
		{
			SetData_Internal(name, &data);
		}

	private:
		void* GetData_Internal(const char* name);
		void SetData_Internal(const char* name, void* data);

	private:
		Ref<Shader> m_Shader = nullptr;
		char* m_Buffer = nullptr;
		size_t m_BufferSizeInBytes = 0;
		eastl::hash_map<uint32_t, Ref<Texture2D>> m_Textures;

		static Ref<Texture2D> s_WhiteTexture;
	};
}
