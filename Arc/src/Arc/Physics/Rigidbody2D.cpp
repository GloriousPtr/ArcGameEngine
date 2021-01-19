#include "arcpch.h"
#include "Rigidbody2D.h"

#include "Physics2D.h"

namespace ArcEngine
{
	Rigidbody2D::Rigidbody2D(const glm::vec2& position, const float rotation, const Rigidbody2DSpecification& specification)
	{
		m_Position.Set(position.x, position.y);
		m_Rotation = rotation;

		b2BodyDef bodyDef;
		bodyDef.position = m_Position;
		
		m_Body2D = Physics2D::GetWorld()->CreateBody(&bodyDef);
		m_Body2D->SetTransform(m_Position, m_Rotation);

		SetSpecification(specification);
	}

	void const Rigidbody2D::SetSpecification(const Rigidbody2DSpecification& specification)
	{
		m_Specification = CreateRef<Rigidbody2DSpecification>(specification);

		SetType(m_Specification->Type);
		SetLinearDamping(m_Specification->LinearDamping);
		SetAngularDamping(m_Specification->AngularDamping);
		SetGravityScale(m_Specification->GravityScale);
		SetCollisionDetection(m_Specification->CollisionDetection);
		SetSleepingMode(m_Specification->SleepingMode);
		SetFreezeRotation(m_Specification->FreezeRotationZ);
	}

	void Rigidbody2D::SetRuntimeTransform(const glm::vec2& position, const float rotation)
	{
		ARC_CORE_ASSERT(m_Body2D, "Body2D is not set!");

		m_Position.Set(position.x, position.y);
		m_Rotation = rotation;
		m_Body2D->SetTransform(m_Position, m_Rotation);
	}
}
