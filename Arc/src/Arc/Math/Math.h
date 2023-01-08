#pragma once

#include <glm/glm.hpp>

namespace ArcEngine
{
	template<typename T>
	concept NumericType = requires(T param)
	{
		requires std::is_integral_v<T> || std::is_floating_point_v<T>;
		requires !std::is_same_v<bool, T>;
		requires std::is_arithmetic_v<decltype(param + 1)>;
		requires !std::is_pointer_v<T>;
	};

	class Math
	{
	public:
		const float EPSILON = 1.17549435E-38f;

		static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);
		
		[[nodiscard]] static float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
		[[nodiscard]] static glm::vec2 SmoothDamp(const glm::vec2& current, const glm::vec2& target, glm::vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
		[[nodiscard]] static glm::vec3 SmoothDamp(const glm::vec3& current, const glm::vec3& target, glm::vec3& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

		[[nodiscard]] static float InverseLerp(float a, float b, float value)
		{
			float den = b - a;
			if (den == 0.0f)
				return 0.0f;
			return (value - a) / den;
		}

		[[nodiscard]] static float InverseLerpClamped(float a, float b, float value)
		{
			float den = b - a;
			if (den == 0.0f)
				return 0.0f;
			return glm::clamp((value - a) / den, 0.0f, 1.0f);
		}
	};
}
