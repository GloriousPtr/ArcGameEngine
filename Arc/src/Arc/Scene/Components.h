#pragma once

#include "Arc/Core/UUID.h"
#include "Arc/Renderer/Texture.h"
#include "Arc/Scene/SceneCamera.h"
#include "Arc/Renderer/Buffer.h"
#include "Arc/Renderer/VertexArray.h"

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>

namespace ArcEngine
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const uint64_t id)
			: ID(id) {}
		operator uint64_t() { return ID; }
	};

	struct TagComponent
	{
		std::string Tag;
		bool renaming = false;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};
	
	class Entity;
	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		UUID Parent = 0;
		std::vector<UUID> Children;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;
		std::string TextureFilepath;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
		void SetTexture(std::string& filepath)
		{
			Texture = Texture2D::Create(filepath);
			TextureFilepath = filepath;
		}
		void RemoveTexture()
		{
			Texture = nullptr;
			TextureFilepath = "";
		}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	class ScriptableEntity;
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);
		
		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	struct MeshComponent
	{
		std::string Filepath;
		Ref<VertexArray> VertexArray;

		enum class CullModeType { Unknown = -1, Front, Back, DoubleSided };

		CullModeType CullMode = CullModeType::Back;

		glm::vec4 AlbedoColor = glm::vec4(1.0f);
		float NormalStrength = 0.5f;
		float Metallic = 0.5f;
		float Roughness = 0.5f;
		float AO = 0.5f;
		glm::vec3 EmissiveColor = glm::vec3(0.0f);
		float EmissiveIntensity = 5.0f;

		bool UseAlbedoMap = false;
		bool UseNormalMap = false;
		bool UseMetallicMap = false;
		bool UseRoughnessMap = false;
		bool UseOcclusionMap = false;
		bool UseEmissiveMap = false;

		Ref<Texture2D> AlbedoMap = nullptr;
		Ref<Texture2D> NormalMap = nullptr;
		Ref<Texture2D> MetallicMap = nullptr;
		Ref<Texture2D> RoughnessMap = nullptr;
		Ref<Texture2D> AmbientOcclusionMap = nullptr;
		Ref<Texture2D> EmissiveMap = nullptr;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
	};

	struct SkyLightComponent
	{
		Ref<TextureCubemap> Texture = nullptr;
		float Intensity = 3.0f;

		SkyLightComponent() = default;
		SkyLightComponent(const SkyLightComponent&) = default;
		void SetTexture(std::string& filepath)
		{
			Texture = TextureCubemap::Create(filepath);
		}
		void RemoveTexture()
		{
			Texture = nullptr;
		}
	};

	struct LightComponent
	{
		enum class LightType { Directional = 0, Point };

		LightType Type;
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 20.0f;

		float Constant = 1.0f;
		float Linear = 0.09f;
		float Quadratic = 0.032f;

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
	};
}
