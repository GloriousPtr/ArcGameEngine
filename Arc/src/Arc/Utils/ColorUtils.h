#pragma once

#include <glm/glm.hpp>

namespace ArcEngine
{
	class ColorUtils
	{
	public:
		static void TempratureToColor(const uint32_t kelvin, glm::vec3& outColor)
		{
			ARC_PROFILE_SCOPE();

			uint32_t temp = glm::clamp(kelvin, 1000u, 40000u) / 100;
			if (temp <= 66)
			{
				outColor = glm::vec3(255.0f,
					99.4708025861f * glm::log(temp) - 161.1195681661f,
					temp <= 19 ? 0.0f : 138.5177312231f * glm::log(temp - 10) - 305.0447927307f) / 255.0f;
			}
			else
			{
				outColor = glm::vec3(329.698727447f * glm::pow(temp - 60, -0.1332047592f),
					288.1221695283f * glm::pow(temp - 60, -0.0755148492f),
					255.0f) / 255.0f;
			}

			outColor = glm::clamp(outColor, glm::vec3(0.0f), glm::vec3(1.0f));
		}
	};
}
