#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Arc/Audio/AudioListener.h"
#include "Arc/Audio/AudioSource.h"
#include "Arc/Core/UUID.h"
#include "Arc/Renderer/Framebuffer.h"
#include "Arc/Renderer/Mesh.h"
#include "Arc/Renderer/ParticleSystem.h"
#include "Arc/Renderer/Texture.h"
#include "Arc/Scene/SceneCamera.h"

namespace ArcEngine
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const uint64_t id)
			: ID(id) {}
		operator uint64_t() { return ID; }
	};

	struct TagComponent
	{
		eastl::string Tag;
		uint16_t Layer = BIT(1);
		bool Enabled = true;

		bool handled = true;

		TagComponent() = default;
		explicit TagComponent(const eastl::string& tag)
			: Tag(tag) {}
	};
	
	class Entity;
	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
	};

	struct RelationshipComponent
	{
		UUID Parent = 0;
		eastl::vector<UUID> Children;
	};

	struct PrefabComponent
	{
		UUID ID;
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;
		glm::vec2 Tiling = glm::vec2(1.0f);
		glm::vec2 Offset = glm::vec2(0.0f);
		int32_t SortingOrder = 0;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// 3D ///////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct MeshComponent
	{
		Ref<Mesh> MeshGeometry = nullptr;
		size_t SubmeshIndex = 0;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Lights ///////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct SkyLightComponent
	{
		Ref<TextureCube> Texture = nullptr;
		float Intensity = 0.7f;
		float Rotation = 0.0f;
	};

	struct LightComponent
	{
		enum class LightType : uint8_t { Directional = 0, Point, Spot };
		enum class ShadowQualityType : uint8_t { Hard = 0, Soft, UltraSoft };

		Ref<Framebuffer> ShadowMapFramebuffer;

		uint32_t Temperature = 6570;
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 20.0f;

		float Range = 1.0f;
		float CutOffAngle = glm::radians(12.5f);
		float OuterCutOffAngle = glm::radians(17.5f);
		
		LightType Type = LightType::Point;
		ShadowQualityType ShadowQuality = ShadowQualityType::UltraSoft;
		bool UseColorTemperatureMode = false;

		LightComponent()
		{
			const FramebufferSpecification spec{ 2048, 2048, { FramebufferTextureFormat::Depth } };
			ShadowMapFramebuffer = Framebuffer::Create(spec);
		}
	};

	struct ParticleSystemComponent
	{
		Ref<ParticleSystem> System = nullptr;

		ParticleSystemComponent()
			: System(CreateRef<ParticleSystem>())
		{
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Rigid body and Collider (2D) /////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct Rigidbody2DComponent
	{
		enum class BodyType : uint8_t { Static = 0, Kinematic, Dynamic };
		
		void* RuntimeBody = nullptr;

		// For interpolation/extrapolation
		glm::vec3 PreviousTranslationRotation = glm::vec3(0.0f);
		glm::vec3 TranslationRotation = glm::vec3(0.0f);

		BodyType Type = BodyType::Dynamic;
		float Mass = 1.0f;
		float LinearDrag = 0.0f;
		float AngularDrag = 0.05f;
		float GravityScale = 1.0f;
		uint8_t AutoMass : 1 = 1;
		uint8_t AllowSleep : 1 = 1;
		uint8_t Awake : 1 = 1;
		uint8_t Continuous : 1 = 0;
		uint8_t Interpolation : 1 = 1;
		uint8_t FreezeRotation : 1 = 0;
	};

	struct BoxCollider2DComponent
	{
		void* RuntimeFixture = nullptr;

		glm::vec2 Size = { 0.5f, 0.5f };
		glm::vec2 Offset = { 0.0f, 0.0f };

		float Density = 1.0f;

		float Friction = 0.2f;
		float Restitution = 0.0f;

		bool IsSensor = false;
	};

	struct CircleCollider2DComponent
	{
		void* RuntimeFixture = nullptr;

		float Radius = 0.5f;
		glm::vec2 Offset = { 0.0f, 0.0f };

		float Density = 1.0f;

		float Friction = 0.2f;
		float Restitution = 0.0f;

		bool IsSensor = false;
	};

	struct PolygonCollider2DComponent
	{
		void* RuntimeFixture = nullptr;

		glm::vec2 Offset = { 0.0f, 0.0f };
		eastl::vector<glm::vec2> Points = { { 0.0f, 0.0f }, { 1.0, 0.0f }, { 0.0f, 1.0f } };

		float Density = 1.0f;

		float Friction = 0.2f;
		float Restitution = 0.0f;

		bool IsSensor = false;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Joints ///////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct DistanceJoint2DComponent
	{
		void* RuntimeJoint = nullptr;

		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);
		glm::vec2 ConnectedAnchor = glm::vec2(0.0f);

		float Distance = 0.0f;
		float MinDistance = 0.0f;
		float MaxDistanceBy = 2.0f;
		float BreakForce = FLT_MAX;

		bool AutoDistance = true;
		bool EnableCollision = false;
	};

	struct SpringJoint2DComponent
	{
		void* RuntimeJoint = nullptr;

		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);
		glm::vec2 ConnectedAnchor = glm::vec2(0.0f);

		float Distance = 0.0f;
		float MinDistance = 0.0f;
		float MaxDistanceBy = 2.0f;
		float Frequency = 4.0f;
		float DampingRatio = 0.5f;
		float BreakForce = FLT_MAX;

		bool AutoDistance = true;
		bool EnableCollision = false;
	};

	struct HingeJoint2DComponent
	{
		void* RuntimeJoint = nullptr;

		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);

		float LowerAngle = glm::radians(0.0f);
		float UpperAngle = glm::radians(359.0f);
		float MotorSpeed = 5.0f;
		float MaxMotorTorque = 10000.0f;
		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;
		
		bool UseLimits = false;
		bool UseMotor = false;
		bool EnableCollision = false;
	};

	struct SliderJoint2DComponent
	{
		void* RuntimeJoint = nullptr;

		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);

		float Angle = 0.0f;
		float LowerTranslation = 0.0f;
		float UpperTranslation = 0.0f;
		float MotorSpeed = 5.0f;
		float MaxMotorForce = 20.0f;
		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;

		bool UseLimits = false;
		bool UseMotor = false;
		bool EnableCollision = false;
	};

	struct WheelJoint2DComponent
	{
		void* RuntimeJoint = nullptr;
		
		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);

		float Frequency = 4.0f;
		float DampingRatio = 0.7f;
		float LowerTranslation = -0.25f;
		float UpperTranslation = 0.25f;
		float MotorSpeed = 10.0f;
		float MaxMotorTorque = 20.0f;
		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;

		bool UseLimits = true;
		bool UseMotor = true;
		bool EnableCollision = false;
	};

	struct BuoyancyEffector2DComponent
	{
		float Density = 2.0f;
		float DragMultiplier = 1.0f;
		float FlowMagnitude = 0.0f;
		float FlowAngle = glm::radians(0.0f);

		bool FlipGravity = false;
	};

	struct RigidbodyComponent
	{
		enum class BodyType : uint8_t { Static = 0, Kinematic, Dynamic };

		void* RuntimeBody = nullptr;

		// For interpolation/extrapolation
		glm::vec3 PreviousTranslation = glm::vec3(0.0f);
		glm::quat PreviousRotation = glm::vec3(0.0f);
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::quat Rotation = glm::vec3(0.0f);

		float Mass = 1.0f;
		float LinearDrag = 0.0f;
		float AngularDrag = 0.05f;
		float GravityScale = 1.0f;
		uint8_t AutoMass : 1 = 1;
		uint8_t AllowSleep : 1 = 1;
		uint8_t Awake : 1 = 1;
		uint8_t Continuous : 1 = 0;
		uint8_t Interpolation : 1 = 1;
		uint8_t IsSensor : 1 = 0;
		BodyType Type = BodyType::Dynamic;
	};

	struct BoxColliderComponent
	{
		glm::vec3 Size = { 0.5f, 0.5f, 0.5f };
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
	};

	struct SphereColliderComponent
	{
		float Radius = 0.5f;
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
	};

	struct CapsuleColliderComponent
	{
		float Height = 1.0f;
		float Radius = 0.5f;
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
	};

	struct TaperedCapsuleColliderComponent
	{
		float Height = 1.0f;
		float TopRadius = 0.5f;
		float BottomRadius = 0.5f;
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
	};

	struct CylinderColliderComponent
	{
		float Height = 1.0f;
		float Radius = 0.5f;
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Script ///////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	class ScriptInstance;

	struct ScriptComponent
	{
		eastl::vector<eastl::string> Classes;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Audio ////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct AudioSourceComponent
	{
		AudioSourceConfig Config;

		Ref<AudioSource> Source = nullptr;
	};

	struct AudioListenerComponent
	{
		bool Active = true;
		AudioListenerConfig Config;

		Ref<AudioListener> Listener;
	};

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents = ComponentGroup<
		TransformComponent,
		RelationshipComponent,
		PrefabComponent,
		CameraComponent,

		// 2D
		SpriteRendererComponent,

		// 3D
		MeshComponent,
		SkyLightComponent,
		LightComponent,

		ParticleSystemComponent,

		// 2D Physics
		Rigidbody2DComponent,
		BoxCollider2DComponent,
		CircleCollider2DComponent,
		PolygonCollider2DComponent,
		DistanceJoint2DComponent,
		SpringJoint2DComponent,
		HingeJoint2DComponent,
		SliderJoint2DComponent,
		WheelJoint2DComponent,
		BuoyancyEffector2DComponent,

		// 3D Physics
		RigidbodyComponent,
		BoxColliderComponent,
		SphereColliderComponent,
		CapsuleColliderComponent,
		TaperedCapsuleColliderComponent,
		CylinderColliderComponent,

		ScriptComponent,

		// Audio
		AudioSourceComponent,
		AudioListenerComponent
	>;
}
