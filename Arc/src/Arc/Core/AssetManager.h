#pragma once

#include <future>
#include <EASTL/hash_map.h>
#include <EASTL/vector.h>

namespace ArcEngine
{
	class Mesh;
	class TextureCubemap;
	class Texture2D;

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
		static eastl::vector<std::future<void>> m_Futures;
	};
}
