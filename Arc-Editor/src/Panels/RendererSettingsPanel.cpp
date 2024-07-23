#include "RendererSettingsPanel.h"

#include <Icons.h>

#include "../Utils/UI.h"

namespace ArcEngine
{
	RendererSettingsPanel::RendererSettingsPanel(const char* name)
		: BasePanel(name, ARC_ICON_GPU)
	{
	}

	void RendererSettingsPanel::OnImGuiRender(WorkQueue* queue)
	{
		ARC_PROFILE_SCOPE();
		if (OnBegin())
		{
			UI::BeginProperties();
			{
				UI::Property("Exposure", Renderer3D::Exposure, 0.0f, 0.0f, "Sets the exposure value.");
				UI::PropertyEnum<Renderer3D::TonemappingType>("Tonemapping", Renderer3D::Tonemapping, "Choose the desired tonemapping algorithm.");
			}
			UI::EndProperties();

			constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

			if (ImGui::TreeNodeEx("Bloom", treeNodeFlags))
			{
				UI::BeginProperties();

				UI::Property("Use Bloom", Renderer3D::UseBloom, "Enable/Disable Bloom.");

				if (UI::Property("Bloom Strength", Renderer3D::BloomStrength, 0.0f, 0.0f, "Strength of the Bloom filter.", 0.01f) && Renderer3D::BloomStrength <= 0.001f)
					Renderer3D::BloomStrength = 0.001f;

				if (UI::Property("Bloom Threshold", Renderer3D::BloomThreshold, 0.0f, 0.0f, "Filters out pixels under this level of brightness. This value is expressed in gamma-space.", 0.01f) && Renderer3D::BloomThreshold <= 0.001f)
					Renderer3D::BloomThreshold = 0.001f;

				UI::Property("Bloom Knee", Renderer3D::BloomKnee, 0.0f, 1.0f, "Makes transition between under/over-threshold gradual (0 = hard threshold, 1 = soft threshold).");

				if (UI::Property("Bloom Clamp", Renderer3D::BloomClamp, 0.0f, 0.0f, "Clamps pixels to control the bloom amount. This value is expressed in gamma-space.", 1.0f) && Renderer3D::BloomClamp <= 0.01f)
					Renderer3D::BloomClamp = 0.01f;

				UI::EndProperties();

				ImGui::TreePop();
			}

#if 0

			if (ImGui::TreeNodeEx("FXAA", treeNodeFlags))
			{
				UI::BeginProperties();
				UI::Property("Use FXAA", Renderer3D::UseFXAA, "Enable/Disable FXAA.");
				UI::Property("Current Threshold", Renderer3D::FXAAThreshold.x, 0.0078125f, 0.0833f, "Trims the algorithm from processing darks (lower = better quality).", 0.1f, "%.4f");
				UI::Property("Relative Threshold", Renderer3D::FXAAThreshold.y, 0.063f, 0.3330f, "The minimum amount of local contrast required to apply (lower = better quality).", 0.1f, "%.4f");
				UI::EndProperties();

				ImGui::TreePop();
			}
#endif

			if (ImGui::TreeNodeEx("Vignette", treeNodeFlags))
			{
				UI::BeginProperties();

				bool vignetteEnabled = Renderer3D::VignetteOffset.w > 0.0f;
				if (UI::Property("Use Vignette", vignetteEnabled, "Enable/Disable Vignette."))
					Renderer3D::VignetteOffset.w = vignetteEnabled ? 1.0f : 0.0f;

				UI::PropertyVector("Color", Renderer3D::VignetteColor, true, false, "Color of the vignette.");
				UI::Property("Intensity", Renderer3D::VignetteColor.a, 0.0f, 50.0f, "Strength of the vignette effect.");

				bool useMask = Renderer3D::VignetteOffset.z > 0.0f;
				if (UI::Property("Use Mask", useMask, "Use a texture mask to create a custom vignette effect."))
					Renderer3D::VignetteOffset.z = useMask ? 1.0f : 0.0f;

				if (useMask)
				{
					UI::Property("Mask", Renderer3D::VignetteMask, 0, "Texture mask for vignette effect");
				}
				else
				{
					glm::vec2 offset = Renderer3D::VignetteOffset;
					if (UI::PropertyVector("Offset", offset, false, false, "Set the offset of the vignette."))
					{
						Renderer3D::VignetteOffset.x = offset.x;
						Renderer3D::VignetteOffset.y = offset.y;
					}
				}
				
				UI::EndProperties();

				ImGui::TreePop();
			}

			OnEnd();
		}
	}
}
