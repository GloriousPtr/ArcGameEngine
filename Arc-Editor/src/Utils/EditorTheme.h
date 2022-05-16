#pragma once

#include <imgui/imgui.h>
#include <glm/glm.hpp>

#include "IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class EditorTheme
	{
		// 0xRRGGBBAA
		inline static int BackGroundColor = 0x25213100;
		inline static int TextColor = 0xF4F1DE00;
		inline static int MainColor = 0xDA115E00;
		inline static int MainAccentColor = 0x79235900;
		inline static int HighlightColor = 0xC7EF0000;

		inline static int Black				= 0x00000000;
		inline static int White				= 0xFFFFFF00;

		inline static int AlphaTransparent	= 0x00;
		inline static int Alpha20			= 0x33;
		inline static int Alpha40			= 0x66;
		inline static int Alpha50			= 0x80;
		inline static int Alpha60			= 0x99;
		inline static int Alpha80			= 0xCC;
		inline static int Alpha90			= 0xE6;
		inline static int AlphaFull			= 0xFF;

		static float GetR(int colorCode) { return (float)((colorCode & 0xFF000000) >> 24 ) / (float)(0xFF); }
		static float GetG(int colorCode) { return (float)((colorCode & 0x00FF0000) >> 16 ) / (float)(0xFF); }
		static float GetB(int colorCode) { return (float)((colorCode & 0x0000FF00) >> 8	 ) / (float)(0xFF); }
		static float GetA(int alphaCode) { return ((float)alphaCode / (float)0xFF); }

		static ImVec4 GetColor(int c, int a = Alpha80)	{ return ImVec4(GetR(c), GetG(c), GetB(c), GetA(a)); }
		static ImVec4 Darken(ImVec4 c, float p)			{ return ImVec4(glm::max(0.f, c.x - 1.0f * p), glm::max(0.f, c.y - 1.0f * p), glm::max(0.f, c.z - 1.0f *p), c.w); }
		static ImVec4 Lighten(ImVec4 c, float p)		{ return ImVec4(glm::max(0.f, c.x + 1.0f * p), glm::max(0.f, c.y + 1.0f * p), glm::max(0.f, c.z + 1.0f *p), c.w); }

		static ImVec4 Disabled(ImVec4 c)	{ return Darken(c, 0.6f); }
		static ImVec4 Hovered(ImVec4 c)		{ return Lighten(c, 0.2f); }
		static ImVec4 Active(ImVec4 c)		{ return Lighten(ImVec4(c.x, c.y, c.z, 1.0f),0.1f); }
		static ImVec4 Collapsed(ImVec4 c)	{ return Darken(c, 0.2f); }

	public:

		static void SetColors(int backGroundColor, int textColor, int mainColor, int mainAccentColor, int highlightColor)
		{
			BackGroundColor = backGroundColor;
			TextColor = textColor;
			MainColor = mainColor;
			MainAccentColor = mainAccentColor;
			HighlightColor = highlightColor;

			ApplyTheme();
		}

		static void ApplyTheme()
		{
			ImVec4* colors = ImGui::GetStyle().Colors;

			colors[ImGuiCol_Text] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.22f, 0.24f, 0.25f, 1.00f);
			colors[ImGuiCol_Border] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.15f, 0.16f, 1.00f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
			colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.22f, 0.23f, 0.25f, 1.00f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.2f, 0.21f, 0.23f, 1.00f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.84f, 0.34f, 0.17f, 0.14f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.28f, 0.14f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
			colors[ImGuiCol_DockingPreview] = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);
			colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.184f, 0.184f, 0.184f, 1.00f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

			ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_Right;

			HeaderSelectedColor = colors[ImGuiCol_HeaderActive];
			WindowBgColor = colors[ImGuiCol_WindowBg];
			WindowBgAlternativeColor = Lighten(WindowBgColor, 0.04f);
			AssetIconColor = Lighten(colors[ImGuiCol_HeaderActive], 0.9f);
		}

		static void SetStyle()
		{
			ImGuiStyle* style = &ImGui::GetStyle();

			style->WindowPadding = ImVec2(8.0f, 8.0f);
			style->FramePadding = ImVec2(4.0f, 3.0f);
			style->CellPadding = ImVec2(4.0f, 2.0f);
			style->ItemSpacing = ImVec2(8.0f, 4.0f);
			style->IndentSpacing = 12;
			style->ScrollbarSize = 14;
			style->GrabMinSize = 10;

			style->WindowBorderSize = 1.0f;
			style->ChildBorderSize = 1.0f;
			style->PopupBorderSize = 3.0f;
			style->FrameBorderSize = 1.0f;
			style->TabBorderSize = 0.0f;

			style->WindowRounding = 6.0f;
			style->ChildRounding = 0.0f;
			style->FrameRounding = 0.0f;
			style->PopupRounding = 0.0f;
			style->ScrollbarRounding = 9.0f;
			style->GrabRounding = 2.0f;
			style->TabRounding = 4.0f;

			style->WindowMenuButtonPosition = ImGuiDir_Right;
			style->ColorButtonPosition = ImGuiDir_Left;
		}

		static void SetFont();

		static ImVec4 HeaderSelectedColor;
		static ImVec4 WindowBgColor;
		static ImVec4 WindowBgAlternativeColor;
		static ImVec4 AssetIconColor;
	};
}
