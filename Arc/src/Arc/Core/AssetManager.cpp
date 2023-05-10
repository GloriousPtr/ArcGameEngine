#include "arcpch.h"
#include "AssetManager.h"

#include "Arc/Core/Application.h"
#include "Arc/Renderer/Mesh.h"
#include "Arc/Renderer/Texture.h"
#include "Arc/Utils/StringUtils.h"

namespace ArcEngine
{
	inline static Ref<Texture2D> s_WhiteTexture;
	inline static Ref<Texture2D> s_BlackTexture;
	inline static eastl::hash_map<eastl::string, Ref<Texture2D>> m_Texture2DMap;
	inline static eastl::hash_map<eastl::string, Ref<TextureCube>> m_TextureCubeMap;
	inline static eastl::hash_map<eastl::string, Ref<Mesh>> m_MeshMap;
	inline static eastl::vector<std::future<void>> m_Futures;

	void AssetManager::Init()
	{
		ARC_PROFILE_SCOPE();

		s_WhiteTexture = Texture2D::Create(1, 1, TextureFormat::RGBA8);
		uint32_t whiteTextureData = 0xffffffff;
		s_WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_BlackTexture = Texture2D::Create(1, 1, TextureFormat::RGBA8);
		uint32_t blackTextureData = 0x000000ff;
		s_BlackTexture->SetData(&blackTextureData, sizeof(uint32_t));
	}

	void AssetManager::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		s_WhiteTexture.reset();
		s_BlackTexture.reset();

		m_Texture2DMap.clear();
		m_TextureCubeMap.clear();
		m_MeshMap.clear();
		m_Futures.clear();
	}

	const Ref<Texture2D>& AssetManager::WhiteTexture()
	{
		return s_WhiteTexture;
	}

	const Ref<Texture2D>& AssetManager::BlackTexture()
	{
		return s_BlackTexture;
	}

	Ref<Texture2D>& AssetManager::GetTexture2D(const eastl::string& path)
	{
		ARC_PROFILE_SCOPE();

		const auto it = m_Texture2DMap.find_as(path);
		if (it != m_Texture2DMap.end())
			return it->second;

		Ref<Texture2D> texture = Texture2D::Create(path, TextureFormat::RGBA8);
		m_Texture2DMap.emplace(path, texture);
		return m_Texture2DMap.at(path);
	}

	Ref<TextureCube>& AssetManager::GetTextureCube(const eastl::string& path)
	{
		ARC_PROFILE_SCOPE();

		const auto it = m_TextureCubeMap.find_as(path);
		if (it != m_TextureCubeMap.end())
			return it->second;

		Ref<TextureCube> texture = TextureCube::Create(path, TextureFormat::RGBA16F);
		m_TextureCubeMap.emplace(path, texture);
		return m_TextureCubeMap.at(path);
	}

	Ref<Mesh>& AssetManager::GetMesh(const eastl::string& path)
	{
		ARC_PROFILE_SCOPE();

		const auto it = m_MeshMap.find_as(path);
		if (it != m_MeshMap.end())
			return it->second;

		Ref<Mesh> mesh = CreateRef<Mesh>(path.c_str());
		m_MeshMap.emplace(path, mesh);
		return m_MeshMap.at(path);
	}
}
