#pragma once

namespace ArcEngine
{
#define IM_COL32_R_SHIFT    0
#define IM_COL32_G_SHIFT    8
#define IM_COL32_B_SHIFT    16
#define IM_COL32_A_SHIFT    24

	struct Color
	{
		uint32_t rgba = 0xFFFFFFFF;
	};

	struct ColorF
	{
		glm::vec4 rgba = glm::vec4(1.0f);
	};

	static inline float Saturate(float f) { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }

	[[maybe_unused]]
	static Color ColorFromColorF(const ColorF& in)
	{
		glm::vec4 rgba = in.rgba;
		uint32_t out;
		out = ((uint32_t)((int)(Saturate(rgba.r) * 255.0f + 0.5f))) << IM_COL32_R_SHIFT;
		out |= ((uint32_t)((int)(Saturate(rgba.g) * 255.0f + 0.5f))) << IM_COL32_G_SHIFT;
		out |= ((uint32_t)((int)(Saturate(rgba.b) * 255.0f + 0.5f))) << IM_COL32_B_SHIFT;
		out |= ((uint32_t)((int)(Saturate(rgba.a) * 255.0f + 0.5f))) << IM_COL32_A_SHIFT;
		return Color{ out };
	}

	[[maybe_unused]]
	static ColorF ColorFFromColor(Color in)
	{
		float s = 1.0f / 255.0f;
		return ColorF(
			glm::vec4(((in.rgba >> IM_COL32_R_SHIFT) & 0xFF) * s,
			((in.rgba >> IM_COL32_G_SHIFT) & 0xFF) * s,
			((in.rgba >> IM_COL32_B_SHIFT) & 0xFF) * s,
			((in.rgba >> IM_COL32_A_SHIFT) & 0xFF) * s));
	}

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
				const float tempX = static_cast<float>(temp - 60);
				outColor = glm::dvec3(329.698727447 * glm::pow(tempX, -0.1332047592),
					288.1221695283 * glm::pow(tempX, -0.0755148492),
					255.0) / 255.0;
			}

			outColor = glm::clamp(outColor, glm::vec3(0.0f), glm::vec3(1.0f));
		}
	};
}
