#include "RendererSettingsPanel.h"

#include "../Utils/UI.h"

#include <imgui/imgui.h>

namespace ArcEngine
{
	RendererSettingsPanel::RendererSettingsPanel()
	{
	}
	
	RendererSettingsPanel::~RendererSettingsPanel()
	{
	}

	void RendererSettingsPanel::OnImGuiRender()
	{
		ImGui::Begin("Renderer Settings");

		UI::PushID();

		UI::BeginPropertyGrid();
		UI::Property("Exposure", Renderer3D::Exposure);
		UI::EndPropertyGrid();

		UI::Property("Tonemapping");
		const char* tonemapTypeStrings[] = { "None", "ACES", "Filmic", "Uncharted" };
		const char* currentTonemapTypeString = tonemapTypeStrings[(int)Renderer3D::Tonemapping];
		if(ImGui::BeginCombo("##Tonemapping", currentTonemapTypeString))
		{
			for (int i = 0; i < 4; i++)
			{
				bool isSelected = currentTonemapTypeString == tonemapTypeStrings[i];
				if(ImGui::Selectable(tonemapTypeStrings[i], isSelected))
				{
					currentTonemapTypeString = tonemapTypeStrings[i];
					Renderer3D::Tonemapping = (Renderer3D::TonemappingType)i;
				}

				if(isSelected)
					ImGui::SetItemDefaultFocus();
			}
			
			ImGui::EndCombo();
		}

		UI::BeginPropertyGrid();
		UI::Property("Use Bloom", Renderer3D::UseBloom);
		UI::EndPropertyGrid();

		UI::BeginPropertyGrid();
		float bloomStrength = Renderer3D::BloomStrength;
		if (UI::Property("Bloom Strength", bloomStrength, 0.01f))
		{
			if (bloomStrength > 0.001f)
				Renderer3D::BloomStrength = bloomStrength;
		}
		UI::EndPropertyGrid();

		UI::BeginPropertyGrid();
		float bloomThreshold = Renderer3D::BloomThreshold;
		if (UI::Property("Bloom Threshold", bloomThreshold, 0.1f))
		{
			if (bloomThreshold > 0.001f)
				Renderer3D::BloomThreshold = bloomThreshold;
		}
		UI::EndPropertyGrid();

		UI::BeginPropertyGrid();
		UI::Property("Bloom Knee", Renderer3D::BloomKnee, 0.0f, 1.0f);
		UI::EndPropertyGrid();

		UI::BeginPropertyGrid();
		float bloomClamp = Renderer3D::BloomClamp;
		if (UI::Property("Bloom Clamp", bloomClamp, 1.0f))
		{
			if (bloomClamp > 0.01f)
				Renderer3D::BloomClamp = bloomClamp;
		}
		UI::EndPropertyGrid();

		int sampleCount = Renderer3D::blurSamples;
		static int selectedDownsampled = 0;
		static int selectedUpsampled = 0;
		static uint32_t textureID = 0;
		ImGui::Image(reinterpret_cast<void*>(Renderer3D::prefilteredFramebuffer->GetColorAttachmentRendererID()), { 256, 256 }, {0, 1}, {1, 0});

		if (ImGui::SliderInt("Downsampled Prefilters", &selectedDownsampled, 0, sampleCount - 1))
			textureID = Renderer3D::downsampledFramebuffers[selectedDownsampled]->GetColorAttachmentRendererID();
		if (ImGui::SliderInt("Upsampled Prefilters", &selectedUpsampled, 0, sampleCount - 1))
			textureID = Renderer3D::upsampledFramebuffers[selectedUpsampled]->GetColorAttachmentRendererID();

		ImGui::Image(reinterpret_cast<void*>(textureID), { 256, 256 }, {0, 1}, {1, 0});

		UI::PopID();

		ImGui::End();
	}
}
