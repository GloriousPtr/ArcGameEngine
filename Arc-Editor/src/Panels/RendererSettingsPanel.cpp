#include "RendererSettingsPanel.h"

#include "../Utils/UI.h"

#include <imgui/imgui.h>

namespace ArcEngine
{
	void RendererSettingsPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		if (OnBegin())
		{
			UI::BeginProperties();
			{
				UI::Property("Exposure", Renderer3D::Exposure, 0.0f, 0.0f, "Sets the exposure value.");

				const char* tonemapTypeStrings[] = { "None", "ACES", "Filmic", "Uncharted" };
				int index = (int)Renderer3D::Tonemapping;
				if (UI::Property("Tonemapping", index, tonemapTypeStrings, 4, "Choose the desired tonemapping algorithm."))
					Renderer3D::Tonemapping = (Renderer3D::TonemappingType)index;
			}
			UI::EndProperties();

			if (ImGui::TreeNode("Bloom"))
			{
				UI::BeginProperties();

				UI::Property("Use Bloom", Renderer3D::UseBloom, "Enable/Disable Bloom.");

				if (UI::Property("Bloom Strength", Renderer3D::BloomStrength, 0.0f, 0.0f, "	Strength of the Bloom filter.", 0.01f) && Renderer3D::BloomStrength <= 0.001f)
					Renderer3D::BloomStrength = 0.001f;

				if (UI::Property("Bloom Threshold", Renderer3D::BloomThreshold, 0.0f, 0.0f, "Filters out pixels under this level of brightness. This value is expressed in gamma-space.", 0.01f) && Renderer3D::BloomThreshold <= 0.001f)
					Renderer3D::BloomThreshold = 0.001f;

				UI::Property("Bloom Knee", Renderer3D::BloomKnee, 0.0f, 1.0f, "Makes transition between under/over-threshold gradual (0 = hard threshold, 1 = soft threshold).");

				if (UI::Property("Bloom Clamp", Renderer3D::BloomClamp, 0.0f, 0.0f, "Clamps pixels to control the bloom amount. This value is expressed in gamma-space.", 1.0f) && Renderer3D::BloomClamp <= 0.01f)
					Renderer3D::BloomClamp = 0.01f;

				UI::EndProperties();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("FXAA"))
			{
				UI::BeginProperties();
				UI::Property("Use FXAA", Renderer3D::UseFXAA, "Enable/Disable FXAA.");
				UI::Property("Current Threshold", Renderer3D::FXAAThreshold.x, 0.0078125f, 0.0833f, "Trims the algorithm from processing darks (lower = better quality).", 0.1f, "%.4f");
				UI::Property("Relative Threshold", Renderer3D::FXAAThreshold.y, 0.063f, 0.3330f, "The minimum amount of local contrast required to apply (lower = better quality).", 0.1f, "%.4f");
				UI::EndProperties();

				ImGui::TreePop();
			}

			OnEnd();
		}
	}
}
