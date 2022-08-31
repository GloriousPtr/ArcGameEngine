#pragma once

#include <box2d/box2d.h>
#include <EASTL/vector.h>
#include <glm/glm.hpp>

namespace ArcEngine
{
	class PhysicsUtils
	{
	public:
		static bool Inside(b2Vec2 cp1, b2Vec2 cp2, b2Vec2 p)
		{
			return (cp2.x - cp1.x) * (p.y - cp1.y) > (cp2.y - cp1.y) * (p.x - cp1.x);
		}

		static b2Vec2 Intersection(b2Vec2 cp1, b2Vec2 cp2, b2Vec2 s, b2Vec2 e)
		{
			b2Vec2 dc(cp1.x - cp2.x, cp1.y - cp2.y);
			b2Vec2 dp(s.x - e.x, s.y - e.y);
			float n1 = cp1.x * cp2.y - cp1.y * cp2.x;
			float n2 = s.x * e.y - s.y * e.x;
			float n3 = 1.0 / (dc.x * dp.y - dc.y * dp.x);
			return b2Vec2((n1 * dp.x - n2 * dc.x) * n3, (n1 * dp.y - n2 * dc.y) * n3);
		}

		static bool VerticesFromCircle(b2Fixture* fixture, eastl::vector<b2Vec2>& vertices, uint32_t resolution = 16)
		{
			if (fixture->GetShape()->GetType() != b2Shape::e_circle)
				return false;

			b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();
			b2Vec2 position = fixture->GetBody()->GetPosition();
			float radius = circle->m_radius;

			uint32_t polyCount = resolution * radius;
			float deltaAngle = 360.0f / polyCount;

			for (uint32_t i = 0; i < polyCount; ++i)
			{
				float radians = glm::radians(deltaAngle * i);
				b2Vec2 point = { glm::cos(radians), glm::sin(radians) };
				vertices.push_back(position + (radius * point));
			}

			return true;
		}

		//http://rosettacode.org/wiki/Sutherland-Hodgman_polygon_clipping#JavaScript
		//Note that this only works when fB is a convex polygon, but we know all 
		//fixtures in Box2D are convex, so that will not be a problem
		static bool FindIntersectionOfFixtures(b2Fixture* fA, b2Fixture* fB, eastl::vector<b2Vec2>& outputVertices)
		{
			eastl::vector<b2Vec2> clipPolygon;

			b2PolygonShape* polyA = nullptr;
			switch (fA->GetShape()->GetType())
			{
			case b2Shape::e_polygon:
				polyA = (b2PolygonShape*)fA->GetShape();
				//fill subject polygon from fixtureA polygon
				for (int i = 0; i < polyA->m_count; i++)
					outputVertices.push_back(fA->GetBody()->GetWorldPoint(polyA->m_vertices[i]));
				break;
			case b2Shape::e_circle:
				VerticesFromCircle(fA, outputVertices);
				break;
			default:
				return false;
			}

			b2PolygonShape* polyB = nullptr;
			switch (fB->GetShape()->GetType())
			{
			case b2Shape::e_polygon:
				polyB = (b2PolygonShape*)fB->GetShape();
				//fill clip polygon from fixtureB polygon
				for (int i = 0; i < polyB->m_count; i++)
					clipPolygon.push_back(fB->GetBody()->GetWorldPoint(polyB->m_vertices[i]));
				break;
			case b2Shape::e_circle:
				VerticesFromCircle(fB, clipPolygon);
				break;
			default:
				return false;
			}

			if (clipPolygon.empty())
				return false;

			b2Vec2 cp1 = clipPolygon[clipPolygon.size() - 1];
			for (int j = 0; j < clipPolygon.size(); j++)
			{
				b2Vec2 cp2 = clipPolygon[j];
				if (outputVertices.empty())
					return false;
				eastl::vector<b2Vec2> inputList = outputVertices;
				outputVertices.clear();
				b2Vec2 s = inputList[inputList.size() - 1]; //last on the input list
				for (int i = 0; i < inputList.size(); i++)
				{
					b2Vec2 e = inputList[i];
					if (Inside(cp1, cp2, e))
					{
						if (!Inside(cp1, cp2, s))
						{
							outputVertices.push_back(Intersection(cp1, cp2, s, e));
						}
						outputVertices.push_back(e);
					}
					else if (Inside(cp1, cp2, s))
					{
						outputVertices.push_back(Intersection(cp1, cp2, s, e));
					}
					s = e;
				}
				cp1 = cp2;
			}

			return !outputVertices.empty();
		}

		static b2Vec2 ComputeCentroid(eastl::vector<b2Vec2> vs, float& area)
		{
			int count = (int)vs.size();
			b2Assert(count >= 3);

			b2Vec2 c;
			c.Set(0.0f, 0.0f);
			area = 0.0f;

			// pRef is the reference point for forming triangles.
			// Its location doesnt change the result (except for rounding error).
			b2Vec2 pRef(0.0f, 0.0f);

			const float inv3 = 1.0f / 3.0f;

			for (int32 i = 0; i < count; ++i)
			{
				// Triangle vertices.
				b2Vec2 p1 = pRef;
				b2Vec2 p2 = vs[i];
				b2Vec2 p3 = i + 1 < count ? vs[i + 1] : vs[0];

				b2Vec2 e1 = p2 - p1;
				b2Vec2 e2 = p3 - p1;

				float D = b2Cross(e1, e2);

				float triangleArea = 0.5f * D;
				area += triangleArea;

				// Area weighted centroid
				c += triangleArea * inv3 * (p1 + p2 + p3);
			}

			// Centroid
			if (area > b2_epsilon)
				c *= 1.0f / area;
			else
				area = 0;
			return c;
		}

		static void HandleBuoyancy(b2Fixture* fluid, b2Fixture* fixture, b2Vec2 gravity, float density, float dragMultiplier)
		{
			eastl::vector<b2Vec2> intersectionPoints;
			if (FindIntersectionOfFixtures(fluid, fixture, intersectionPoints))
			{
				float area = 0;
				b2Vec2 centroid = ComputeCentroid(intersectionPoints, area);

				float displacedMass = density * area;
				b2Vec2 buoyancyForce = displacedMass * -gravity;
				fixture->GetBody()->ApplyForce(buoyancyForce, centroid, true);

				//apply drag separately for each polygon edge
				for (int i = 0; i < intersectionPoints.size(); i++)
				{
					//the end points and mid-point of this edge 
					b2Vec2 v0 = intersectionPoints[i];
					b2Vec2 v1 = intersectionPoints[(i + 1) % intersectionPoints.size()];
					b2Vec2 midPoint = 0.5f * (v0 + v1);

					//find relative velocity between object and fluid at edge midpoint
					b2Vec2 velDir = fixture->GetBody()->GetLinearVelocityFromWorldPoint(midPoint) -
						fluid->GetBody()->GetLinearVelocityFromWorldPoint(midPoint);

					b2Vec2 edge = v1 - v0;
					b2Vec2 normal = b2Cross(-1, edge); //gets perpendicular vector

					float dragDot = b2Dot(normal, velDir);
					if (dragDot < 0)
						continue; //normal points backwards - this is not a leading edge

					float vel = velDir.Normalize();
					float edgeLength = edge.Normalize();

					float dragMag = dragDot * edgeLength * density * vel * vel;
					b2Vec2 dragForce = dragMag * dragMultiplier * -velDir;
					fixture->GetBody()->ApplyForce(dragForce, midPoint, true);

					//apply lift
					float liftMag = b2Dot(edge, velDir) * dragMag;
					b2Vec2 liftDir = b2Cross(1, velDir); //gets perpendicular vector
					b2Vec2 liftForce = liftMag * liftDir;
					fixture->GetBody()->ApplyForce(liftForce, midPoint, true);
				}
			}
		}
	};
}
