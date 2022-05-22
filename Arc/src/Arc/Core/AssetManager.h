#pragma once

#include <EASTL/unordered_map.h>

#include "Arc/Renderer/Texture.h"

namespace ArcEngine
{
	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<Texture2D> GetTexture2D(eastl::string path);
		static Ref<TextureCubemap> GetTextureCubemap(eastl::string path);

	private:
		static eastl::unordered_map<eastl::string, Ref<Texture2D>> m_Texture2DMap;
		static eastl::unordered_map<eastl::string, Ref<TextureCubemap>> m_TextureCubeMap;
	};
}
