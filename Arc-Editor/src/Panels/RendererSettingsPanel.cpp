#include "RendererSettingsPanel.h"

#include "../Utils/UI.h"

#include <imgui/imgui.h>

namespace ArcEngine
{
	void RendererSettingsPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin(m_ID.c_str(), &m_Showing);

		UI::BeginProperties();
		UI::Property("Exposure", Renderer3D::Exposure);
		const char* tonemapTypeStrings[] = { "None", "ACES", "Filmic", "Uncharted" };
		int index = (int)Renderer3D::Tonemapping;
		if (UI::Property("Tonemapping", index, tonemapTypeStrings, 4))
		{
			Renderer3D::Tonemapping = (Renderer3D::TonemappingType)index;
		}
		UI::EndProperties();

		if (ImGui::TreeNode("Bloom"))
		{
			UI::BeginProperties();

			UI::Property("Use Bloom", Renderer3D::UseBloom);
			
			float bloomStrength = Renderer3D::BloomStrength;
			if (UI::Property("Bloom Strength", bloomStrength, 0.01f))
			{
				if (bloomStrength > 0.001f)
					Renderer3D::BloomStrength = bloomStrength;
			}

			float bloomThreshold = Renderer3D::BloomThreshold;
			if (UI::Property("Bloom Threshold", bloomThreshold, 0.1f))
			{
				if (bloomThreshold > 0.001f)
					Renderer3D::BloomThreshold = bloomThreshold;
			}

			UI::Property("Bloom Knee", Renderer3D::BloomKnee, 0.0f, 1.0f);

			float bloomClamp = Renderer3D::BloomClamp;
			if (UI::Property("Bloom Clamp", bloomClamp, 1.0f))
				if (bloomClamp > 0.01f)
					Renderer3D::BloomClamp = bloomClamp;

			/*
			int sampleCount = Renderer3D::blurSamples;
			static int selectedDownsampled = 0;
			static int selectedUpsampled = 0;
			static uint32_t textureID = 0;
			ImGui::Image(reinterpret_cast<void*>(Renderer3D::prefilteredFramebuffer->GetColorAttachmentRendererID()), { 256, 256 }, {0, 1}, {1, 0});

			if (ImGui::SliderInt("Downsampled Prefilters", &selectedDownsampled, 0, sampleCount - 1))
				textureID = Renderer3D::downsampledFramebuffers[selectedDownsampled]->GetColorAttachmentRendererID();
			if (ImGui::SliderInt("Upsampled Prefilters", &selectedUpsampled, 0, sampleCount - 1))
				textureID = Renderer3D::upsampledFramebuffers[selectedUpsampled]->GetColorAttachmentRendererID();
				*/
			UI::EndProperties();

			ImGui::TreePop();
		}

		ImGui::End();
	}
}
