#include "arcpch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace ArcEngine
{
	bool Math::DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		// From glm::decompose in matrix_decompose.inl
		ARC_PROFILE_SCOPE()

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], 0.0f, epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}

		return true;
	}

	float Math::InverseLerp(float a, float b, float value)
	{
		float den = b - a;
		if (den == 0.0f)
			return 0.0f;
		return (value - a) / den;
	}

	float Math::Lerp(float a, float b, float t)
	{
		return (1.0f - t) * a + t * b;
	}

	float Math::InverseLerpClamped(float a, float b, float value)
	{
		float den = b - a;
		if (den == 0.0f)
			return 0.0f;
		return glm::clamp((value - a) / den, 0.0f, 1.0f);
	}

	float Math::LerpClamped(float a, float b, float t)
	{
		return glm::clamp((1.0f - t)* a + t * b, 0.0f, 1.0f);
	}

	// Gradually changes a value towards a desired goal over time.
	float Math::SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
	{
		// Based on Game Programming Gems 4 Chapter 1.10
		smoothTime = glm::max(0.0001F, smoothTime);
		float omega = 2.0f / smoothTime;

		float x = omega * deltaTime;
		float exp = 1.0f / (1.0f + x + 0.48F * x * x + 0.235F * x * x * x);
		float change = current - target;
		float originalTo = target;

		// Clamp maximum speed
		float maxChange = maxSpeed * smoothTime;
		change = glm::clamp(change, -maxChange, maxChange);
		target = current - change;

		float temp = (currentVelocity + omega * change) * deltaTime;
		currentVelocity = (currentVelocity - omega * temp) * exp;
		float output = target + (change + temp) * exp;

		// Prevent overshooting
		if (originalTo - current > 0.0F == output > originalTo)
		{
			output = originalTo;
			currentVelocity = (output - originalTo) / deltaTime;
		}

		return output;
	}

	glm::vec2 Math::SmoothDamp(const glm::vec2& current, const glm::vec2& target, glm::vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
	{
		// Based on Game Programming Gems 4 Chapter 1.10
		smoothTime = glm::max(0.0001F, smoothTime);
		float omega = 2.0f / smoothTime;

		float x = omega * deltaTime;
		float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

		float change_x = current.x - target.x;
		float change_y = current.y - target.y;
		glm::vec2 originalTo = target;

		// Clamp maximum speed
		float maxChange = maxSpeed * smoothTime;

		float maxChangeSq = maxChange * maxChange;
		float sqDist = change_x * change_x + change_y * change_y;
		if (sqDist > maxChangeSq)
		{
			float mag = glm::sqrt(sqDist);
			change_x = change_x / mag * maxChange;
			change_y = change_y / mag * maxChange;
		}

		float newTargetX = current.x - change_x;
		float newTargetY = current.y - change_y;

		float temp_x = (currentVelocity.x + omega * change_x) * deltaTime;
		float temp_y = (currentVelocity.y + omega * change_y) * deltaTime;

		currentVelocity.x = (currentVelocity.x - omega * temp_x) * exp;
		currentVelocity.y = (currentVelocity.y - omega * temp_y) * exp;

		float output_x = newTargetX + (change_x + temp_x) * exp;
		float output_y = newTargetY + (change_y + temp_y) * exp;

		// Prevent overshooting
		float origMinusCurrent_x = originalTo.x - current.x;
		float origMinusCurrent_y = originalTo.y - current.y;
		float outMinusOrig_x = output_x - originalTo.x;
		float outMinusOrig_y = output_y - originalTo.y;

		if (origMinusCurrent_x * outMinusOrig_x + origMinusCurrent_y * outMinusOrig_y > 0)
		{
			output_x = originalTo.x;
			output_y = originalTo.y;

			currentVelocity.x = (output_x - originalTo.x) / deltaTime;
			currentVelocity.y = (output_y - originalTo.y) / deltaTime;
		}
		return { output_x, output_y };
	}

	// Gradually changes a vector towards a desired goal over time.
	glm::vec3 Math::SmoothDamp(const glm::vec3& current, const glm::vec3& target, glm::vec3& currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
	{
		// Based on Game Programming Gems 4 Chapter 1.10
		smoothTime = glm::max(0.0001F, smoothTime);
		float omega = 2.0f / smoothTime;

		float x = omega * deltaTime;
		float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

		float change_x = current.x - target.x;
		float change_y = current.y - target.y;
		float change_z = current.z - target.z;
		glm::vec3 originalTo = target;

		// Clamp maximum speed
		float maxChange = maxSpeed * smoothTime;

		float maxChangeSq = maxChange * maxChange;
		float sqrmag = change_x * change_x + change_y * change_y + change_z * change_z;
		if (sqrmag > maxChangeSq)
		{
			float mag = glm::sqrt(sqrmag);
			change_x = change_x / mag * maxChange;
			change_y = change_y / mag * maxChange;
			change_z = change_z / mag * maxChange;
		}

		float newTargetX = current.x - change_x;
		float newTargetY = current.y - change_y;
		float newTargetZ = current.z - change_z;

		float temp_x = (currentVelocity.x + omega * change_x) * deltaTime;
		float temp_y = (currentVelocity.y + omega * change_y) * deltaTime;
		float temp_z = (currentVelocity.z + omega * change_z) * deltaTime;

		currentVelocity.x = (currentVelocity.x - omega * temp_x) * exp;
		currentVelocity.y = (currentVelocity.y - omega * temp_y) * exp;
		currentVelocity.z = (currentVelocity.z - omega * temp_z) * exp;

		float output_x = newTargetX + (change_x + temp_x) * exp;
		float output_y = newTargetY + (change_y + temp_y) * exp;
		float output_z = newTargetZ + (change_z + temp_z) * exp;

		// Prevent overshooting
		float origMinusCurrent_x = originalTo.x - current.x;
		float origMinusCurrent_y = originalTo.y - current.y;
		float origMinusCurrent_z = originalTo.z - current.z;
		float outMinusOrig_x = output_x - originalTo.x;
		float outMinusOrig_y = output_y - originalTo.y;
		float outMinusOrig_z = output_z - originalTo.z;

		if (origMinusCurrent_x * outMinusOrig_x + origMinusCurrent_y * outMinusOrig_y + origMinusCurrent_z * outMinusOrig_z > 0)
		{
			output_x = originalTo.x;
			output_y = originalTo.y;
			output_z = originalTo.z;

			currentVelocity.x = (output_x - originalTo.x) / deltaTime;
			currentVelocity.y = (output_y - originalTo.y) / deltaTime;
			currentVelocity.z = (output_z - originalTo.z) / deltaTime;
		}

		return { output_x, output_y, output_z };
	}
}
