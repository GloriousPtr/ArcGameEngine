#pragma once

#include <glm/glm.hpp>

namespace ArcEngine
{
	class Math
	{
	public:
		const float EPSILON = 1.17549435E-38f;

		static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);
		static float InverseLerp(float a, float b, float value);
		static float Lerp(float a, float b, float t);
		static float InverseLerpClamped(float a, float b, float value);
		static float LerpClamped(float a, float b, float t);
		static float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
		static glm::vec2 SmoothDamp(const glm::vec2& current, const glm::vec2& target, glm::vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
		static glm::vec3 SmoothDamp(const glm::vec3& current, const glm::vec3& target, glm::vec3& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
	};
}
