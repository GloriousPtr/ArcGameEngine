#include "arcpch.h"
#include "AssetManager.h"

#include <stb_image.h>

#include "Arc/Core/Application.h"
#include "Arc/Renderer/Mesh.h"
#include "Arc/Renderer/Texture.h"
#include "Arc/Utils/StringUtils.h"

namespace ArcEngine
{
	inline static Ref<Texture2D> s_WhiteTexture;
	inline static Ref<Texture2D> s_BlackTexture;
	inline static std::unordered_map<std::string, Ref<Texture2D>, UM_StringTransparentEquality> m_Texture2DMap;
	inline static std::unordered_map<std::string, Ref<TextureCubemap>, UM_StringTransparentEquality> m_TextureCubeMap;
	inline static std::unordered_map<std::string, Ref<Mesh>, UM_StringTransparentEquality> m_MeshMap;
	inline static std::vector<std::future<void>> m_Futures;

	void AssetManager::Init()
	{
		ARC_PROFILE_SCOPE()

		s_WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_BlackTexture = Texture2D::Create(1, 1);
		uint32_t blackTextureData = 0x000000ff;
		s_BlackTexture->SetData(&blackTextureData, sizeof(uint32_t));
	}

	void AssetManager::Shutdown()
	{
		ARC_PROFILE_SCOPE()

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

	static void LoadTexture2D(Texture2D* tex, const std::string_view path)
	{
		ARC_PROFILE_THREAD("IO Thread")

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			ARC_PROFILE_SCOPE("stbi_load Texture")

			data = stbi_load(path.data(), &width, &height, &channels, 0);
		}
		ARC_CORE_ASSERT(data, "Failed to load image!")
		Application::Get().SubmitToMainThread([tex, path, width, height, data, channels]() { tex->Invalidate(path, width, height, data, channels); stbi_image_free(data); });
	}

	Ref<Texture2D>& AssetManager::GetTexture2D(const std::string& path)
	{
		ARC_PROFILE_SCOPE()

		const auto& it = m_Texture2DMap.find(path);
		if (it != m_Texture2DMap.end())
			return it->second;

		Ref<Texture2D> texture = Texture2D::Create(path);
		m_Texture2DMap.emplace(path, texture);
		//m_Futures.push_back(std::async(std::launch::async, &LoadTexture2D, texture.get(), path));
		return m_Texture2DMap[path];
	}

	static void LoadTextureCubemap(TextureCubemap* tex, const std::string_view path)
	{
		ARC_PROFILE_THREAD("IO Thread")

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		float* data = nullptr;
		{
			ARC_PROFILE_SCOPE("stbi_load Texture")

			data = stbi_loadf(path.data(), &width, &height, &channels, 0);
		}
		ARC_CORE_ASSERT(data, "Failed to load image!")
		Application::Get().SubmitToMainThread([tex, path, width, height, data, channels]() { tex->Invalidate(path, width, height, data, channels); stbi_image_free(data); });
	}

	Ref<TextureCubemap>& AssetManager::GetTextureCubemap(const std::string& path)
	{
		ARC_PROFILE_SCOPE()

		const auto& it = m_TextureCubeMap.find(path);
		if (it != m_TextureCubeMap.end())
			return it->second;

		Ref<TextureCubemap> texture = TextureCubemap::Create();
		m_TextureCubeMap.emplace(path, texture);
		m_Futures.push_back(std::async(std::launch::async, &LoadTextureCubemap, texture.get(), path));
		return m_TextureCubeMap[path];
	}

	Ref<Mesh>& AssetManager::GetMesh(const std::string& path)
	{
		ARC_PROFILE_SCOPE()

		const auto& it = m_MeshMap.find(path);
		if (it != m_MeshMap.end())
			return it->second;

		Ref<Mesh> mesh = CreateRef<Mesh>(path.c_str());
		m_MeshMap.emplace(path, mesh);
		return m_MeshMap[path];
	}
}
