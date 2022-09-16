#pragma once

#include <glm/glm.hpp>

namespace ArcEngine::Math
{
	constexpr float EPSILON = 1.17549435E-38f;
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);
	float InverseLerp(float a, float b, float value);
	float Lerp(float a, float b, float t);
	float InverseLerpClamped(float a, float b, float value);
	float LerpClamped(float a, float b, float t);
}
