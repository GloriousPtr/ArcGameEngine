#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


#include "Arc/Renderer/Texture.h"
#include "Arc/Physics/Rigidbody2D.h"
#include "Arc/Physics/BoxCollider2D.h"
#include "Arc/Scene/SceneCamera.h"
#include "Arc/Scene/ScriptableEntity.h"

namespace ArcEngine
{
	struct IDComponent
	{
		uint32_t ID = 0;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const uint32_t id)
			: ID(id) {}
		operator uint32_t() { return ID; }
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
	
	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), Translation) * glm::toMat4(glm::quat(Rotation)) * glm::scale(glm::mat4(1.0f), Scale);
		}
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

	struct Rigidbody2DComponent
	{
		Rigidbody2D::Rigidbody2DSpecification Specification;
		Ref<Rigidbody2D> Body2D;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;

		void StartSimulation(const glm::vec2& translation, const float rotation)
		{
			Body2D = CreateRef<Rigidbody2D>(translation, rotation, Specification);
		}
		
		void ValidateSpecification()
		{
			if (Body2D)
				Body2D->SetSpecification(Specification);
		}
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Size{ 1.0f, 1.0f };
		glm::vec2 Offset{ 0.0f, 0.0f };
		bool IsTrigger = false;

		Ref<BoxCollider2D> Collider2D;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;

		void StartSimulation(Ref<Rigidbody2D>& rigidbody2D)
		{
			Collider2D = CreateRef<BoxCollider2D>(rigidbody2D, Size, Offset, IsTrigger);
		}

		void ValidateSpecification()
		{
			if (Collider2D)
				Collider2D->SetSpecification(Size, Offset, IsTrigger);
		}
	};
}
