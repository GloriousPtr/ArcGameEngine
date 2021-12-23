#include "RendererSettingsPanel.h"

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

		uint32_t textureID = Renderer3D::GetShadowMapTextureID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ 256, 256 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		ImGui::End();
	}
}
