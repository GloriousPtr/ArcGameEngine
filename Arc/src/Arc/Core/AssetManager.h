#pragma once

#include "Arc/Renderer/Texture.h"

namespace ArcEngine
{
	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<Texture2D> GetTexture2D(std::string path);
		static Ref<TextureCubemap> GetTextureCubemap(std::string path);

	private:
		static std::unordered_map<std::string, Ref<Texture2D>> m_Texture2DMap;
		static std::unordered_map<std::string, Ref<TextureCubemap>> m_TextureCubeMap;
	};
}
