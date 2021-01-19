#pragma once

#include "Rigidbody2D.h"

#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_fixture.h>

#include <glm/glm.hpp>

namespace ArcEngine
{
	class BoxCollider2D
	{
	public:
		BoxCollider2D(Ref<Rigidbody2D>& rigidbody2D, glm::vec2& size, glm::vec2& offset, bool isTrigger);
		~BoxCollider2D();

		void SetSpecification(glm::vec2& size, glm::vec2& offset, bool isTrigger);

	private:
		void CreateFixture(Ref<Rigidbody2D>& rigidbody2D, glm::vec2& size, glm::vec2& offset, bool isTrigger);

	private:
		b2Vec2 m_Size = b2Vec2(1.0f, 1.0f);
		b2Vec2 m_Offset = b2Vec2(0.0f, 0.0f);
		bool m_IsTrigger = false;
		
		Ref<Rigidbody2D> m_Rigidbody2D;
		b2Fixture* m_Fixture;
	};
}

