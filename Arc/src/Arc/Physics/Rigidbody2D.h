#pragma once

#include "Arc/Core/Base.h"

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>
#include <box2d/b2_fixture.h>

#include <glm/glm.hpp>

namespace ArcEngine
{
	enum class Rigidbody2dType
	{
		Static = b2BodyType::b2_staticBody,
		Kinematic = b2BodyType::b2_kinematicBody,
		Dynamic = b2BodyType::b2_dynamicBody
	};

	class Rigidbody2D
	{
	public:
		enum class CollisionDetectionType { Discrete = 0, Continuous };
		enum class SleepType { NeverSleep = 0, StartAwake, StartAsleep };

		struct Rigidbody2DSpecification
		{
			Rigidbody2dType Type = Rigidbody2dType::Dynamic;
			float LinearDamping = 0.0f;
			float AngularDamping = 0.0f;
			float GravityScale = 1.0f;

			CollisionDetectionType CollisionDetection = CollisionDetectionType::Discrete;
			SleepType SleepingMode = SleepType::StartAwake;

			bool FreezeRotationZ = false;
		};

	public:
		Rigidbody2D(const glm::vec2& position, const float rotation, const Rigidbody2DSpecification& specification);
		~Rigidbody2D() = default;

		inline glm::vec2& const GetRuntimePosition() const { ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!"); return (glm::vec2&)m_Body2D->GetPosition(); }
		inline float const GetRuntimeRotation() const { ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!"); return m_Body2D->GetAngle(); }
		void SetRuntimeTransform(const glm::vec2& position, const float rotation);

		
		inline Ref<Rigidbody2DSpecification> const GetSpecification() const { return m_Specification; }
		void const SetSpecification(const Rigidbody2DSpecification& specification);

		// Set properties
		inline void SetType(Rigidbody2dType type) { ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!"); m_Body2D->SetType((b2BodyType)type); }
		inline void SetLinearDamping(float value) { ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!"); m_Body2D->SetLinearDamping(value); }
		inline void SetAngularDamping(float value) { ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!"); m_Body2D->SetAngularDamping(value); }
		inline void SetGravityScale(float value) { ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!"); m_Body2D->SetGravityScale(value); }
		inline void SetCollisionDetection(CollisionDetectionType type)
		{
			ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!");

			switch (type)
			{
				case CollisionDetectionType::Discrete:
				{
					m_Body2D->SetBullet(false);
					break;
				}
				case CollisionDetectionType::Continuous:
				{
					m_Body2D->SetBullet(true);
					break;
				}
			}
		}
		inline void SetSleepingMode(SleepType type)
		{
			ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!");

			switch (type)
			{
				case SleepType::NeverSleep:
				{
					m_Body2D->SetSleepingAllowed(false);
					m_Body2D->SetAwake(true);
					break;
				}
				case SleepType::StartAsleep:
				{
					m_Body2D->SetSleepingAllowed(true);
					m_Body2D->SetAwake(false);
					break;
				}
				case SleepType::StartAwake:
				{
					m_Body2D->SetSleepingAllowed(true);
					m_Body2D->SetAwake(true);
					break;
				}
			}
		}
		inline void SetFreezeRotation(bool flag) { ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!"); m_Body2D->SetFixedRotation(flag); }

	private:
		b2Vec2 m_Position;
		float m_Rotation;

		Ref<Rigidbody2DSpecification> m_Specification;

		b2Body* m_Body2D;

		friend class BoxCollider2D;
	};
}

