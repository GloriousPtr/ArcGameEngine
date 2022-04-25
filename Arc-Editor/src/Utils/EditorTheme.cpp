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
		ImFontConfig iconsConfig;
		// merge in icons from Font Awesome
		iconsConfig.MergeMode = true;
		iconsConfig.PixelSnapH = true;
		iconsConfig.GlyphOffset.y = 1.0f;
		iconsConfig.OversampleH = iconsConfig.OversampleV = 1;
		iconsConfig.GlyphMinAdvanceX = 4.0f;
		iconsConfig.SizePixels = 12.0f;

		io.Fonts->AddFontFromMemoryCompressedTTF(MaterialDesign_compressed_data, MaterialDesign_compressed_size, fontSize, &iconsConfig, icons_ranges);
	}


	void EditorTheme::SetFont()
	{
		ImGuiIO& io = ImGui::GetIO();
		float fontSize = 18.0f;
		
		ImFontConfig iconsConfig;
        iconsConfig.MergeMode = false;
        iconsConfig.PixelSnapH = true;
        iconsConfig.OversampleH = iconsConfig.OversampleV = 1;
        iconsConfig.GlyphMinAdvanceX = 4.0f;
        iconsConfig.SizePixels = 12.0f;
		
		io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", fontSize, &iconsConfig);
		AddIconFont(fontSize);

		io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", fontSize, &iconsConfig);
		AddIconFont(fontSize);

		io.Fonts->TexGlyphPadding = 1;
        for(int n = 0; n < io.Fonts->ConfigData.Size; n++)
        {
            ImFontConfig* fontConfig = (ImFontConfig*)&io.Fonts->ConfigData[n];
            fontConfig->RasterizerMultiply = 1.0f;
        }

		io.Fonts->Build();
	}

}
