#include "arcpch.h"
#include "BoxCollider2D.h"

namespace ArcEngine
{
	BoxCollider2D::BoxCollider2D(Ref<Rigidbody2D>& rigidbody2D, glm::vec2& size, glm::vec2& offset, bool isTrigger)
	{
		CreateFixture(rigidbody2D, size, offset, isTrigger);
	}

	BoxCollider2D::~BoxCollider2D()
	{
	}

	void BoxCollider2D::SetSpecification(glm::vec2& size, glm::vec2& offset, bool isTrigger)
	{
		if (m_Size.x != size.x || m_Size.y != size.y || m_Offset.x != offset.x || m_Offset.y != offset.y)
		{
			m_Size = (b2Vec2&)size;
			m_Offset = (b2Vec2&)offset;
			m_IsTrigger = isTrigger;

			m_Rigidbody2D->m_Body2D->DestroyFixture(m_Fixture);
			CreateFixture(m_Rigidbody2D, size, offset, isTrigger);
		}
		else if(m_IsTrigger != isTrigger)
		{
			m_IsTrigger = isTrigger;
			m_Fixture->SetSensor(m_IsTrigger);
		}
	}

	void BoxCollider2D::CreateFixture(Ref<Rigidbody2D>& rigidbody2D, glm::vec2& size, glm::vec2& offset, bool isTrigger)
	{
		ARC_CORE_ASSERT(rigidbody2D, "Rigidbody2D is null!");

		m_Rigidbody2D = rigidbody2D;
		m_Size.Set(size.x, size.y);
		m_Offset.Set(offset.x, offset.y);
		m_IsTrigger = isTrigger;

		b2PolygonShape box;
		box.SetAsBox(m_Size.x / 2, m_Size.y / 2, m_Offset, 0.0f);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &box;

		if (m_Rigidbody2D->GetSpecification()->Type == Rigidbody2dType::Dynamic)
		{
			fixtureDef.density = 1.0f;
			fixtureDef.friction = 0.3f;
		}
		fixtureDef.isSensor = m_IsTrigger;

		m_Fixture = m_Rigidbody2D->m_Body2D->CreateFixture(&fixtureDef);
	}
}
