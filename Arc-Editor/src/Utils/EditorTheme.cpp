#include "EditorTheme.h"

#include "MaterialDesign.inl"

namespace ArcEngine
{
	ImVec4 EditorTheme::HeaderSelectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 EditorTheme::WindowBgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 EditorTheme::WindowBgAlternativeColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	static void AddIconFont(float fontSize)
	{
		ImGuiIO& io = ImGui::GetIO();

		static const ImWchar icons_ranges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
		ImFontConfig icons_config;
		// merge in icons from Font Awesome
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphOffset.y = 1.0f;
		icons_config.OversampleH = icons_config.OversampleV = 1;
		icons_config.GlyphMinAdvanceX = 4.0f;
		icons_config.SizePixels = 12.0f;

		io.Fonts->AddFontFromMemoryCompressedTTF(MaterialDesign_compressed_data, MaterialDesign_compressed_size, fontSize, &icons_config, icons_ranges);
	}


	void EditorTheme::SetFont()
	{
		ImGuiIO& io = ImGui::GetIO();
		float fontSize = 18.0f;
		
		io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", fontSize);
		AddIconFont(fontSize);
		io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", fontSize);
		AddIconFont(fontSize);

		io.Fonts->Build();
	}

}
