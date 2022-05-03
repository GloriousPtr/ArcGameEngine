#pragma once

#include <glm/glm.hpp>

namespace ArcEngine::Math
{
	constexpr float EPSILON = 1.17549435E-38f;
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);
}

