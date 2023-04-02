#pragma once

namespace ArcEngine
{
	class ColorUtils
	{
	public:
		static void TempratureToColor(const uint32_t kelvin, glm::vec3& outColor)
		{
			ARC_PROFILE_SCOPE();

			const uint32_t temp = glm::clamp(kelvin, 1000u, 40000u) / 100;
			if (temp <= 66)
			{
				outColor = glm::dvec3(255.0,
					99.4708025861 * glm::log(temp) - 161.1195681661,
					temp <= 19 ? 0.0 : 138.5177312231 * glm::log(temp - 10) - 305.0447927307) / 255.0;
			}
			else
			{
				const auto tempX = static_cast<float>(temp - 60);
				outColor = glm::dvec3(329.698727447 * glm::pow(tempX, -0.1332047592),
					288.1221695283 * glm::pow(tempX, -0.0755148492),
					255.0) / 255.0;
			}

			outColor = glm::clamp(outColor, glm::vec3(0.0f), glm::vec3(1.0f));
		}
	};
}
