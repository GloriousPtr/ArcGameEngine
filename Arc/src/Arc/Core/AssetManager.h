#pragma once

#include <EASTL/hash_map.h>

#include "Arc/Renderer/Texture.h"
#include "Arc/Renderer/Mesh.h"

namespace ArcEngine
{
	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<Texture2D> GetTexture2D(const eastl::string& path);
		static Ref<TextureCubemap> GetTextureCubemap(const eastl::string& path);
		static Ref<Mesh> GetMesh(const eastl::string& path);

	private:
		static eastl::hash_map<eastl::string, Ref<Texture2D>> m_Texture2DMap;
		static eastl::hash_map<eastl::string, Ref<TextureCubemap>> m_TextureCubeMap;
		static eastl::hash_map<eastl::string, Ref<Mesh>> m_MeshMap;
	};
}
