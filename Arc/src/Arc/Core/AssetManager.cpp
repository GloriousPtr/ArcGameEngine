#include "arcpch.h"
#include "AssetManager.h"

#include <fstream>
#include <stb_image.h>

#include "Arc/Core/Application.h"
#include "Arc/Renderer/Mesh.h"
#include "Arc/Renderer/Texture.h"

namespace ArcEngine
{
	eastl::hash_map<eastl::string, Ref<Texture2D>> AssetManager::m_Texture2DMap;
	eastl::hash_map<eastl::string, Ref<TextureCubemap>> AssetManager::m_TextureCubeMap;
	eastl::hash_map<eastl::string, Ref<Mesh>> AssetManager::m_MeshMap;
	eastl::vector<std::future<void>> AssetManager::m_Futures;

	void AssetManager::Init()
	{
		ARC_PROFILE_SCOPE();

		/* Init stuff here */
	}

	void AssetManager::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		m_Texture2DMap.clear();
		m_TextureCubeMap.clear();
		m_MeshMap.clear();
		m_Futures.clear();
	}

	static void LoadTexture2D(Texture2D* tex, const eastl::string_view path)
	{
		ARC_PROFILE_THREAD("IO Thread");

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			ARC_PROFILE_SCOPE("stbi_load Texture");

			data = stbi_load(path.data(), &width, &height, &channels, 0);
		}
		ARC_CORE_ASSERT(data, "Failed to load image!");
		Application::Get().SubmitToMainThread([tex, width, height, data, channels]() { tex->Invalidate(width, height, data, channels); stbi_image_free(data); });
	}

	Ref<Texture2D> AssetManager::GetTexture2D(const eastl::string& path)
	{
		ARC_PROFILE_SCOPE();

		if (m_Texture2DMap.find_as(path) != m_Texture2DMap.end())
			return m_Texture2DMap.at(path);

		Ref<Texture2D> texture = Texture2D::Create();
		m_Texture2DMap.emplace(path, texture);
		m_Futures.push_back(std::async(std::launch::async, &LoadTexture2D, texture.get(), path));
		return texture;
	}

	static std::mutex mtx;
	static void LoadTextureCubemap(TextureCubemap* tex, const eastl::string_view path)
	{
		ARC_PROFILE_THREAD("IO Thread");

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		float* data = nullptr;
		{
			ARC_PROFILE_SCOPE("stbi_load Texture");

			data = stbi_loadf(path.data(), &width, &height, &channels, 0);
		}
		ARC_CORE_ASSERT(data, "Failed to load image!");
		Application::Get().SubmitToMainThread([tex, width, height, data, channels]() { tex->Invalidate(width, height, data, channels); stbi_image_free(data); });
	}

	Ref<TextureCubemap> AssetManager::GetTextureCubemap(const eastl::string& path)
	{
		ARC_PROFILE_SCOPE();

		if (m_TextureCubeMap.find_as(path) != m_TextureCubeMap.end())
			return m_TextureCubeMap.at(path);

		Ref<TextureCubemap> texture = TextureCubemap::Create();
		m_TextureCubeMap.emplace(path, texture);
		m_Futures.push_back(std::async(std::launch::async, &LoadTextureCubemap, texture.get(), path));
		return texture;
	}

	Ref<Mesh> AssetManager::GetMesh(const eastl::string& path)
	{
		ARC_PROFILE_SCOPE();

		if (m_MeshMap.find_as(path) != m_MeshMap.end())
			return m_MeshMap.at(path);

		Ref<Mesh> mesh = CreateRef<Mesh>(path.c_str());
		m_MeshMap.emplace(path, mesh);
		return mesh;
	}
}
