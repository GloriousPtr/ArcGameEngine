#pragma once

#include "Arc/Core/UUID.h"
#include "Arc/Renderer/Texture.h"
#include "Arc/Scene/SceneCamera.h"
#include "Arc/Renderer/Buffer.h"
#include "Arc/Renderer/VertexArray.h"
#include "Arc/Renderer/Framebuffer.h"
#include "Arc/Utils/AABB.h"

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
		AABB BoundingBox;

		enum class CullModeType { Unknown = -1, Front, Back, DoubleSided };

		CullModeType CullMode = CullModeType::Back;

		glm::vec4 AlbedoColor = glm::vec4(1.0f);
		float Metallic = 0.0f;
		float Roughness = 1.0f;
		glm::vec4 EmissiveParams = glm::vec4(0.0f);

		bool UseAlbedoMap = false;
		bool UseNormalMap = false;
		bool UseMRAMap = false;
		bool UseEmissiveMap = false;

		Ref<Texture2D> AlbedoMap = nullptr;
		Ref<Texture2D> NormalMap = nullptr;
		Ref<Texture2D> MRAMap = nullptr;
		Ref<Texture2D> EmissiveMap = nullptr;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
	};

	struct SkyLightComponent
	{
		Ref<TextureCubemap> Texture = nullptr;
		float Intensity = 0.7f;
		float Rotation = 0.0f;

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
		enum class LightType { Directional = 0, Point, Spot };

		LightType Type = LightType::Directional;
		bool UseColorTempratureMode = true;
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 20.0f;

		float Range = 1.0f;
		float CutOffAngle = 12.5f;
		float OuterCutOffAngle = 17.5f;
		
		Ref<Framebuffer> ShadowMapFramebuffer;

		LightComponent()
		{
			FramebufferSpecification spec;
			spec.Attachments = { FramebufferTextureFormat::Depth };
			spec.Width = 4096;
			spec.Height = 4096;
			ShadowMapFramebuffer = Framebuffer::Create(spec);

			if (UseColorTempratureMode)
				SetTemprature(m_Temprature);
		}

		LightComponent(const LightComponent&)
		{
			FramebufferSpecification spec;
			spec.Attachments = { FramebufferTextureFormat::Depth };
			spec.Width = 4096;
			spec.Height = 4096;
			ShadowMapFramebuffer = Framebuffer::Create(spec);
		}

		uint32_t GetTemprature() { return m_Temprature; }

		void SetTemprature(const uint32_t kelvin)
		{
			m_Temprature = glm::clamp(kelvin, 1000u, 40000u);
			
			uint32_t temp = m_Temprature / 100;
			if (temp <= 66)
			{
				Color = glm::vec3(255.0f,
								  99.4708025861f * glm::log(temp) - 161.1195681661f,
								  temp <= 19 ? 0.0f : 138.5177312231f * glm::log(temp - 10) - 305.0447927307f) / 255.0f;
			}
			else
			{
				Color = glm::vec3(329.698727447f * glm::pow(temp - 60, -0.1332047592f),
								  288.1221695283f * glm::pow(temp - 60, -0.0755148492f),
								  255.0f) / 255.0f;
			}

			Color = glm::clamp(Color, glm::vec3(0.0f), glm::vec3(1.0f));
		}

	private:
		uint32_t m_Temprature = 6570;
	};
}
