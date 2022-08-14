#include "arcpch.h"
#include "AssetManager.h"

namespace ArcEngine
{
	eastl::hash_map<eastl::string, Ref<Texture2D>> AssetManager::m_Texture2DMap;
	eastl::hash_map<eastl::string, Ref<TextureCubemap>> AssetManager::m_TextureCubeMap;
	eastl::hash_map<eastl::string, Ref<Mesh>> AssetManager::m_MeshMap;

	void AssetManager::Init()
	{
	}

	void AssetManager::Shutdown()
	{
		m_Texture2DMap.clear();
		m_TextureCubeMap.clear();
	}

	Ref<Texture2D> AssetManager::GetTexture2D(const eastl::string& path)
	{
		if (m_Texture2DMap.find_as(path) != m_Texture2DMap.end())
			return m_Texture2DMap.at(path);

		Ref<Texture2D> texture = Texture2D::Create(path);
		m_Texture2DMap.emplace(path, texture);
		return texture;
	}

	Ref<TextureCubemap> AssetManager::GetTextureCubemap(const eastl::string& path)
	{
		if (m_TextureCubeMap.find_as(path) != m_TextureCubeMap.end())
			return m_TextureCubeMap.at(path);

		Ref<TextureCubemap> texture = TextureCubemap::Create(path);
		m_TextureCubeMap.emplace(path, texture);
		return texture;
	}

	Ref<Mesh> AssetManager::GetMesh(const eastl::string& path)
	{
		if (m_MeshMap.find_as(path) != m_MeshMap.end())
			return m_MeshMap.at(path);

		Ref<Mesh> mesh = CreateRef<Mesh>(path.c_str());
		m_MeshMap.emplace(path, mesh);
		return mesh;
	}
}
