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
		std::string Tag;
		uint16_t Layer = BIT(1);
		bool Enabled = true;

		bool handled = true;

		TagComponent() = default;
		explicit TagComponent(const std::string& tag)
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
		std::vector<UUID> Children;
	};

	struct PrefabComponent
	{
		UUID ID;
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;
		int32_t SortingOrder = 0;
		float TilingFactor = 1.0f;
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
		enum class CullModeType { Unknown = -1, Front, Back, DoubleSided };

		Ref<Mesh> MeshGeometry = nullptr;
		size_t SubmeshIndex = 0;
		CullModeType CullMode = CullModeType::Back;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Lights ///////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	struct SkyLightComponent
	{
		Ref<TextureCubemap> Texture = nullptr;
		float Intensity = 0.7f;
		float Rotation = 0.0f;
	};

	struct LightComponent
	{
		enum class LightType { Directional = 0, Point, Spot };
		enum class ShadowQualityType { Hard = 0, Soft, UltraSoft };

		LightType Type = LightType::Point;
		bool UseColorTemperatureMode = false;
		uint32_t Temperature = 6570;
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 20.0f;

		float Range = 1.0f;
		float CutOffAngle = glm::radians(12.5f);
		float OuterCutOffAngle = glm::radians(17.5f);
		
		ShadowQualityType ShadowQuality = ShadowQualityType::UltraSoft;

		Ref<Framebuffer> ShadowMapFramebuffer;

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
		enum class BodyType { Static = 0, Kinematic, Dynamic };

		BodyType Type = BodyType::Dynamic;
		bool AutoMass = true;
		float Mass = 1.0f;
		float LinearDrag = 0.0f;
		float AngularDrag = 0.05f;
		float GravityScale = 1.0f;
		bool AllowSleep = true;
		bool Awake = true;
		bool Continuous = false;
		bool Interpolation = true;
		bool FreezeRotation = false;

		void* RuntimeBody = nullptr;

		// For interpolation/extrapolation
		glm::vec3 PreviousTranslationRotation = glm::vec3(0.0f);
		glm::vec3 TranslationRotation = glm::vec3(0.0f);
	};

	struct BoxCollider2DComponent
	{
		bool IsSensor = false;
		glm::vec2 Size = { 0.5f, 0.5f };
		glm::vec2 Offset = { 0.0f, 0.0f };

		float Density = 1.0f;

		float Friction = 0.2f;
		float Restitution = 0.0f;

		void* RuntimeFixture = nullptr;
	};

	struct CircleCollider2DComponent
	{
		bool IsSensor = false;
		float Radius = 0.5f;
		glm::vec2 Offset = { 0.0f, 0.0f };

		float Density = 1.0f;

		float Friction = 0.2f;
		float Restitution = 0.0f;

		void* RuntimeFixture = nullptr;
	};

	struct PolygonCollider2DComponent
	{
		bool IsSensor = false;
		glm::vec2 Offset = { 0.0f, 0.0f };
		std::vector<glm::vec2> Points = { { 0.0f, 0.0f }, { 1.0, 0.0f }, { 0.0f, 1.0f } };

		float Density = 1.0f;

		float Friction = 0.2f;
		float Restitution = 0.0f;

		void* RuntimeFixture = nullptr;
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

		bool AutoDistance = true;
		float Distance = 0.0f;
		float MinDistance = 0.0f;
		float MaxDistanceBy = 2.0f;
		float BreakForce = FLT_MAX;

		void* RuntimeJoint = nullptr;
	};

	struct SpringJoint2DComponent
	{
		bool EnableCollision = false;
		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);
		glm::vec2 ConnectedAnchor = glm::vec2(0.0f);

		bool AutoDistance = true;
		float Distance = 0.0f;
		float MinDistance = 0.0f;
		float MaxDistanceBy = 2.0f;
		float Frequency = 4.0f;
		float DampingRatio = 0.5f;
		float BreakForce = FLT_MAX;

		void* RuntimeJoint = nullptr;
	};

	struct HingeJoint2DComponent
	{
		bool EnableCollision = false;
		UUID ConnectedRigidbody = 0;
		glm::vec2 Anchor = glm::vec2(0.0f);

		bool UseLimits = false;
		float LowerAngle = glm::radians(0.0f);
		float UpperAngle = glm::radians(359.0f);
		
		bool UseMotor = false;
		float MotorSpeed = 5.0f;
		float MaxMotorTorque = 10000.0f;

		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;

		void* RuntimeJoint = nullptr;
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
	};

	struct BuoyancyEffector2DComponent
	{
		float Density = 2.0f;
		float DragMultiplier = 1.0f;
		bool FlipGravity = false;

		float FlowMagnitude = 0.0f;
		float FlowAngle = glm::radians(0.0f);
	};

	struct RigidbodyComponent
	{
		enum class BodyType { Static = 0, Kinematic, Dynamic };

		BodyType Type = BodyType::Dynamic;
		bool AutoMass = true;
		float Mass = 1.0f;
		float LinearDrag = 0.0f;
		float AngularDrag = 0.05f;
		float GravityScale = 1.0f;
		bool AllowSleep = true;
		bool Awake = true;
		bool Continuous = false;
		bool Interpolation = true;

		bool IsSensor = false;

		void* RuntimeBody = nullptr;

		// For interpolation/extrapolation
		glm::vec3 PreviousTranslation = glm::vec3(0.0f);
		glm::quat PreviousRotation = glm::vec3(0.0f);
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::quat Rotation = glm::vec3(0.0f);
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
		std::vector<std::string> Classes;
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
