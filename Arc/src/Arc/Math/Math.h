#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/component_wise.hpp>

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
		static constexpr float EPSILON = 1.17549435E-38f;

		static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);
		
		[[nodiscard]] static float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

		template<typename T>
		[[nodiscard]] static T SmoothDamp(const T& current, const T& target, T& currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
		{
			// Based on Game Programming Gems 4 Chapter 1.10
			smoothTime = glm::max(0.0001F, smoothTime);
			const float omega = 2.0f / smoothTime;

			const float x = omega * deltaTime;
			const float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

			T change = current - target;
			const T originalTo = target;

			// Clamp maximum speed
			const float maxChange = maxSpeed * smoothTime;

			const float maxChangeSq = maxChange * maxChange;
			const float sqDist = glm::length2(change);
			if (sqDist > maxChangeSq)
			{
				const float mag = glm::sqrt(sqDist);
				change = change / mag * maxChange;
			}

			const T newTarget = current - change;
			const T temp = (currentVelocity + omega * change) * deltaTime;

			currentVelocity = (currentVelocity - omega * temp) * exp;

			T output = newTarget + (change + temp) * exp;

			// Prevent overshooting
			const T origMinusCurrent = originalTo - current;
			const T outMinusOrig = output - originalTo;

			if (glm::compAdd(origMinusCurrent * outMinusOrig) > 0.0f)
			{
				output = originalTo;
				currentVelocity = (output - originalTo) / deltaTime;
			}
			return output;
		}

		[[nodiscard]] static float InverseLerp(float a, float b, float value)
		{
			const float den = b - a;
			if (den == 0.0f)
				return 0.0f;
			return (value - a) / den;
		}

		[[nodiscard]] static float InverseLerpClamped(float a, float b, float value)
		{
			const float den = b - a;
			if (den == 0.0f)
				return 0.0f;
			return glm::clamp((value - a) / den, 0.0f, 1.0f);
		}
	};
}
