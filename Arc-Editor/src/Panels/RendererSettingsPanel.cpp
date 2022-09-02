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
				UI::Property("Exposure", Renderer3D::Exposure);

				const char* tonemapTypeStrings[] = { "None", "ACES", "Filmic", "Uncharted" };
				int index = (int)Renderer3D::Tonemapping;
				if (UI::Property("Tonemapping", index, tonemapTypeStrings, 4))
					Renderer3D::Tonemapping = (Renderer3D::TonemappingType)index;
			}
			UI::EndProperties();

			if (ImGui::TreeNode("Bloom"))
			{
				UI::BeginProperties();

				UI::Property("Use Bloom", Renderer3D::UseBloom);

				if (UI::Property("Bloom Strength", Renderer3D::BloomStrength, nullptr, 0.01f) && Renderer3D::BloomStrength <= 0.001f)
					Renderer3D::BloomStrength = 0.001f;

				if (UI::Property("Bloom Threshold", Renderer3D::BloomThreshold, nullptr, 0.01f) && Renderer3D::BloomThreshold <= 0.001f)
					Renderer3D::BloomThreshold = 0.001f;

				UI::Property("Bloom Knee", Renderer3D::BloomKnee, 0.0f, 1.0f);

				if (UI::Property("Bloom Clamp", Renderer3D::BloomClamp, nullptr, 1.0f) && Renderer3D::BloomClamp <= 0.01f)
					Renderer3D::BloomClamp = 0.01f;

				UI::EndProperties();

				ImGui::TreePop();
			}

			OnEnd();
		}
	}
}
