#pragma once

#include <glm/glm.hpp>

namespace ArcEngine
{
	struct AABB
	{
		AABB(const glm::vec3& min = glm::vec3(0.0f), const glm::vec3& max = glm::vec3(0.0f))
			: Min(min), Max(max)
		{
		}
		
		glm::vec3 Min;
		glm::vec3 Max;

		glm::vec3 GetPosition() { return glm::vec3(Max.x - Min.x, Max.y - Min.y, Max.z - Min.z) * 0.5f; }
	};
}
