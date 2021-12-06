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
		void RemoveTexture() { Texture = nullptr; }
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
		Ref<Texture2D> DiffuseMap = nullptr;

		MeshComponent() = default;
	};
}
