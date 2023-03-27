#pragma once

namespace ArcEngine
{
	class Mesh;
	class TextureCube;
	class Texture2D;

	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();

		[[nodiscard]] static const Ref<Texture2D>& WhiteTexture();
		[[nodiscard]] static const Ref<Texture2D>& BlackTexture();
		[[nodiscard]] static Ref<Texture2D>& GetTexture2D(const std::string& path);
		[[nodiscard]] static Ref<TextureCube>& GetTextureCube(const std::string& path);
		[[nodiscard]] static Ref<Mesh>& GetMesh(const std::string& path);
	};
}
