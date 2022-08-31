#pragma once

#include "Arc/Core/UUID.h"
#include "Arc/Renderer/Texture.h"
#include "Arc/Scene/SceneCamera.h"
#include "Arc/Renderer/Buffer.h"
#include "Arc/Renderer/Material.h"
#include "Arc/Renderer/VertexArray.h"
#include "Arc/Renderer/Framebuffer.h"
#include "Arc/Renderer/Mesh.h"
#include "Arc/Utils/AABB.h"
#include "Arc/Scripting/Field.h"
#include "Arc/Audio/AudioSource.h"
#include "Arc/Audio/AudioListener.h"

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <EASTL/hash_set.h>

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
		eastl::string Tag;
		bool Enabled = true;

		bool handled = true;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const eastl::string& tag)
			: Tag(tag) {}
	};
	
	class Entity;
	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}
	};

	struct RelationshipComponent
	{
		UUID Parent = 0;
		eastl::vector<UUID> Children;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent&) = default;
	};

	struct PrefabComponent
	{
		UUID ID;

		PrefabComponent() = default;
		PrefabComponent(const PrefabComponent&) = default;
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
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

	/////////////////////////////////////////////////////////////////////////////////////
	// 3D ///////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct MeshComponent
	{
		enum class CullModeType { Unknown = -1, Front, Back, DoubleSided };

		eastl::string Filepath;
		Ref<Mesh> MeshGeometry = nullptr;
		uint32_t SubmeshIndex = 0;
		CullModeType CullMode = CullModeType::Back;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Lights ///////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct SkyLightComponent
	{
		Ref<TextureCubemap> Texture = nullptr;
		float Intensity = 0.7f;
		float Rotation = 0.0f;

		SkyLightComponent() = default;
		SkyLightComponent(const SkyLightComponent&) = default;
	};

	struct LightComponent
	{
		enum class LightType { Directional = 0, Point, Spot };
		enum class ShadowQualityType { Hard = 0, Soft, UltraSoft };

		LightType Type = LightType::Point;
		bool UseColorTempratureMode = true;
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 20.0f;

		float Range = 1.0f;
		float CutOffAngle = 12.5f;
		float OuterCutOffAngle = 17.5f;
		
		ShadowQualityType ShadowQuality = ShadowQualityType::UltraSoft;

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

		LightComponent(const LightComponent& other)
		{
			Type = other.Type;
			UseColorTempratureMode = other.UseColorTempratureMode;
			Color = other.Color;
			Intensity = other.Intensity;
			Range = other.Range;
			CutOffAngle = other.CutOffAngle;
			OuterCutOffAngle = other.OuterCutOffAngle;
			ShadowQuality = other.ShadowQuality;

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

	/////////////////////////////////////////////////////////////////////////////////////
	// Rigidbody and Colliders (2D) /////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Kinematic, Dynamic };

		BodyType Type = BodyType::Dynamic;
		bool AutoMass = true;
		float Mass = 1.0f;
		float LinearDrag = 0.0f;
		float AngularDrag = 0.05f;
		bool AllowSleep = true;
		bool Awake = true;
		bool Continuous = false;
		bool FreezeRotation = false;
		float GravityScale = 1.0f;

		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Size = { 0.5f, 0.5f };
		glm::vec2 Offset = { 0.0f, 0.0f };
		bool IsSensor = false;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		float Radius = 0.5f;
		glm::vec2 Offset = { 0.0f, 0.0f };
		bool IsSensor = false;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Joints ///////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct DistanceJoint2DComponent
	{
		bool EnableCollision = false;
		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);
		glm::vec2 ConnectedAnchor = glm::vec2(0.0f);
		float MinDistance = 0.0f;
		float MaxDistance = 2.0f;
		float BreakForce = FLT_MAX;

		void* RuntimeJoint = nullptr;

		DistanceJoint2DComponent() = default;
		DistanceJoint2DComponent(const DistanceJoint2DComponent&) = default;
	};

	struct SpringJoint2DComponent
	{
		bool EnableCollision = false;
		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);
		glm::vec2 ConnectedAnchor = glm::vec2(0.0f);
		float MinDistance = 0.0f;
		float MaxDistance = 2.0f;
		float Frequency = 4.0f;
		float DampingRatio = 0.5f;
		float BreakForce = FLT_MAX;

		void* RuntimeJoint = nullptr;

		SpringJoint2DComponent() = default;
		SpringJoint2DComponent(const SpringJoint2DComponent&) = default;
	};

	struct HingeJoint2DComponent
	{
		bool EnableCollision = false;
		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);

		bool UseLimits = false;
		float LowerAngle = 0.0f;
		float UpperAngle = 359.0f;
		
		bool UseMotor = false;
		float MotorSpeed = 5.0f;
		float MaxMotorTorque = 10000.0f;

		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;

		void* RuntimeJoint = nullptr;

		HingeJoint2DComponent() = default;
		HingeJoint2DComponent(const HingeJoint2DComponent&) = default;
	};

	struct SliderJoint2DComponent
	{
		bool EnableCollision = false;
		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);
		float Angle = 0.0f;

		bool UseLimits = false;
		float LowerTranslation = 0.0f;
		float UpperTranslation = 0.0f;

		bool UseMotor = false;
		float MotorSpeed = 5.0f;
		float MaxMotorForce = 20.0f;

		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;

		void* RuntimeJoint = nullptr;

		SliderJoint2DComponent() = default;
		SliderJoint2DComponent(const SliderJoint2DComponent&) = default;
	};

	struct WheelJoint2DComponent
	{
		bool EnableCollision = false;
		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);

		float Frequency = 4.0f;
		float DampingRatio = 0.7f;

		bool UseLimits = true;
		float LowerTranslation = -0.25f;
		float UpperTranslation = 0.25f;

		bool UseMotor = true;
		float MotorSpeed = 10.0f;
		float MaxMotorTorque = 20.0f;

		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;

		void* RuntimeJoint = nullptr;

		WheelJoint2DComponent() = default;
		WheelJoint2DComponent(const WheelJoint2DComponent&) = default;
	};

	struct BuoyancyEffector2DComponent
	{
		float Density = 2.0f;
		float DragMultiplier = 1.0f;

		void* RuntimeJoint = nullptr;

		BuoyancyEffector2DComponent() = default;
		BuoyancyEffector2DComponent(const BuoyancyEffector2DComponent&) = default;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Script ///////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	class ScriptInstance;

	struct ScriptComponent
	{
		eastl::vector<eastl::string> Classes;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Audio ////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct AudioSourceComponent
	{
		AudioSourceConfig Config;

		Ref<AudioSource> Source = nullptr;

		AudioSourceComponent() = default;
		AudioSourceComponent(const AudioSourceComponent&) = default;
	};

	struct AudioListenerComponent
	{
		bool Active = true;
		AudioListenerConfig Config;

		Ref<AudioListener> Listener;

		AudioListenerComponent() = default;
		AudioListenerComponent(const AudioListenerComponent&) = default;
	};

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents = ComponentGroup<
		TransformComponent,
		RelationshipComponent,
		PrefabComponent,
		SpriteRendererComponent,
		CameraComponent,
		NativeScriptComponent,
		MeshComponent,
		SkyLightComponent,
		LightComponent,
		Rigidbody2DComponent,
		BoxCollider2DComponent,
		CircleCollider2DComponent,
		DistanceJoint2DComponent,
		SpringJoint2DComponent,
		HingeJoint2DComponent,
		SliderJoint2DComponent,
		WheelJoint2DComponent,
		BuoyancyEffector2DComponent,
		ScriptComponent,
		AudioSourceComponent,
		AudioListenerComponent
	>;
}
