#include "AssetPanel.h"

#include <ArcEngine.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <filesystem>

#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	static const std::filesystem::path s_AssetPath = "assets";

	static const char* GetFileIcon(const char* ext)
	{
		if (!(strcmp(ext, "txt") && strcmp(ext, "md")))
			return ICON_MDI_FILE_DOCUMENT;
		if (!(strcmp(ext, "png") && strcmp(ext, "jpg") && strcmp(ext, "jpeg") && strcmp(ext, "bmp") && strcmp(ext, "gif")))
			return ICON_MDI_FILE_IMAGE;
		if (!(strcmp(ext, "hdr") && strcmp(ext, "tga")))
			return ICON_MDI_IMAGE_FILTER_HDR;
		if (!(strcmp(ext, "glsl")))
			return ICON_MDI_IMAGE_FILTER_BLACK_WHITE;
		if (!(strcmp(ext, "obj") && strcmp(ext, "fbx") && strcmp(ext, "gltf")))
			return ICON_MDI_VECTOR_POLYGON;

		return ICON_MDI_FILE;
	}
	
	std::pair<bool, uint32_t> AssetPanel::DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selectionMask)
	{
		ARC_PROFILE_SCOPE();

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		bool anyNodeClicked = false;
		uint32_t nodeClicked = 0;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			ImGuiTreeNodeFlags nodeFlags = flags;

			const bool selected = (*selectionMask & BIT(*count)) != 0;
			if (selected)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			eastl::string name = StringUtils::GetNameWithExtension(entry.path().string().c_str());
			
			bool entryIsFile = !std::filesystem::is_directory(entry.path());
			if (entryIsFile)
				nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			bool open = ImGui::TreeNodeEx((void*)(intptr_t)(*count), nodeFlags, "");
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				if (!entryIsFile)
					UpdateDirectoryEntries(entry.path());

				nodeClicked = *count;
				anyNodeClicked = true;
			}

			const char* folderIcon;
			if (entryIsFile)
				folderIcon = GetFileIcon(StringUtils::GetExtension((eastl::string&&)name).c_str());
			else
				folderIcon = open ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
			
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
			ImGui::TextUnformatted(folderIcon);
			ImGui::PopStyleColor();
			ImGui::SameLine();
			float x = ImGui::GetContentRegionAvail().x;
			ImGui::TextUnformatted(name.c_str());
			m_CurrentlyVisibleItemsTreeView++;

			(*count)--;

			if (!entryIsFile)
			{
				if (open)
				{
					auto clickState = DirectoryTreeViewRecursive(entry.path(), count, selectionMask);

					if (!anyNodeClicked)
					{
						anyNodeClicked = clickState.first;
						nodeClicked = clickState.second;
					}

					ImGui::TreePop();
				}
				else
				{
					for (const auto& e : std::filesystem::recursive_directory_iterator(entry.path()))
						(*count)--;
				}
			}
		}

		return { anyNodeClicked, nodeClicked };
	}

	AssetPanel::AssetPanel(const char* name)
		: BasePanel(name, ICON_MDI_FOLDER_STAR, true), m_CurrentDirectory(s_AssetPath)
	{
		ARC_PROFILE_SCOPE();

		m_WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		m_WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");

		UpdateDirectoryEntries(s_AssetPath);
	}

	void AssetPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		ImGui::Begin(m_ID.c_str(), &m_Showing);
		{
			RenderHeader();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Columns(2, nullptr, true);
			{
				ImVec2 region = ImGui::GetContentRegionAvail();
				static bool firstTime = true;
				if (!firstTime)
					m_TreeViewColumnWidth = ImGui::GetColumnWidth(0);
				else
					firstTime = false;
				ImGui::SetColumnWidth(0, m_TreeViewColumnWidth);
				ImGui::BeginChild("TreeView");
				{
					RenderSideView();
				}
				ImGui::EndChild();
				ImGui::NextColumn();
				ImGui::Separator();
				ImGui::BeginChild("FolderView");
				{
					RenderBody();
				}
				ImGui::EndChild();
			}
			ImGui::Columns(1);
		}
		ImGui::End();
	}

	void AssetPanel::RenderHeader()
	{
		ARC_PROFILE_SCOPE();

		if (ImGui::Button(ICON_MDI_COGS))
			ImGui::OpenPopup("SettingsPopup");
		if (ImGui::BeginPopup("SettingsPopup"))
		{
			ImGui::SliderFloat("Thumbnail Size", &m_ThumbnailSize, 64.0f, 256.0f);
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_MAGNIFY);
		ImGui::SameLine();

		float spacing = ImGui::GetStyle().ItemSpacing.x;
        ImGui::GetStyle().ItemSpacing.x = 2;

		{
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[1];
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
			m_Filter.Draw("###ConsoleFilter", ImGui::GetContentRegionAvail().x);
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[0];
		}

		ImGui::GetStyle().ItemSpacing.x = spacing;
		
		if(!m_Filter.IsActive())
		{
			ImGui::SameLine();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[1];
			ImGui::SetCursorPosX(ImGui::GetFontSize() * 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));
			ImGui::TextUnformatted("Search...");
			ImGui::PopStyleVar();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[0];
		}

		// Back button
		{
			bool disabledBackButton = false;
			if (m_CurrentDirectory == std::filesystem::path(s_AssetPath))
				disabledBackButton = true;
		
			if (disabledBackButton)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::Button(ICON_MDI_ARROW_LEFT))
				UpdateDirectoryEntries(m_CurrentDirectory.parent_path());

			if (disabledBackButton)
			{
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}
		}
		ImGui::SameLine();
		ImGui::Text(m_CurrentDirectory.string().c_str());
	}

	void AssetPanel::RenderSideView()
	{
		ARC_PROFILE_SCOPE();

		static int selectionMask = 0;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (m_CurrentDirectory == s_AssetPath && selectionMask == 0)
			flags |= ImGuiTreeNodeFlags_Selected;
		
		float x1 = ImGui::GetCurrentWindow()->WorkRect.Min.x;
		float x2 = ImGui::GetCurrentWindow()->WorkRect.Max.x;
		float itemSpacingY = ImGui::GetStyle().ItemSpacing.y;
		float itemOffsetY = -itemSpacingY * 0.5f;
		float lineHeight = ImGui::GetTextLineHeight() + itemSpacingY;
		UI::DrawRowsBackground(m_CurrentlyVisibleItemsTreeView, lineHeight, x1, x2, itemOffsetY, 0, ImGui::GetColorU32(EditorTheme::WindowBgColor));
		
		m_CurrentlyVisibleItemsTreeView = 1;

		bool opened = ImGui::TreeNodeEx((void*)("Assets"), flags, "");
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			UpdateDirectoryEntries(s_AssetPath);
			selectionMask = 0;
		}
		const char* folderIcon = open ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
		ImGui::TextUnformatted(folderIcon);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		float x = ImGui::GetContentRegionAvail().x;
		ImGui::TextUnformatted("Assets");

		if (opened)
		{
			uint32_t count = 0;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(s_AssetPath))
				count++;

			auto clickState = DirectoryTreeViewRecursive(s_AssetPath, &count, &selectionMask);

			if (clickState.first)
			{
				// (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
				if (ImGui::GetIO().KeyCtrl)
					selectionMask ^= BIT(clickState.second);          // CTRL+click to toggle
				else //if (!(selection_mask & (1 << clickState.second))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
					selectionMask = BIT(clickState.second);           // Click to single-select
			}

			ImGui::TreePop();
		}
	}

	void AssetPanel::RenderBody()
	{
		ARC_PROFILE_SCOPE();

		std::filesystem::path directoryToOpen;
		bool directoryOpened = false;

		float padding = 4.0f;
		float cellSize = m_ThumbnailSize + 2 * padding;

		float overlayPaddingY = 6.0f * padding;
		float thumbnailPadding = overlayPaddingY * 0.5f;
		float thumbnailSize = m_ThumbnailSize - thumbnailPadding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int) (panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		uint32_t flags = ImGuiTableFlags_ContextMenuInBody;
		flags |= ImGuiTableFlags_SizingStretchSame;
		flags |= ImGuiTableFlags_NoClip;

		uint32_t i = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
		if (ImGui::BeginTable("BodyTable", columnCount, flags))
		{
			for (auto& file : m_DirectoryEntries)
			{
				ImGui::PushID(i++);

				uint32_t textureId = m_DirectoryIcon->GetRendererID();
				const char* fontIcon = nullptr;
				if (!file.DirectoryEntry.is_directory())
				{
					eastl::string ext = StringUtils::GetExtension((eastl::string&&)file.Name);
					if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
						textureId = AssetManager::GetTexture2D(file.DirectoryEntry.path().string().c_str())->GetRendererID();
					else
						fontIcon = GetFileIcon(ext.c_str());
				}

				ImGui::TableNextColumn();

				const char* filename = file.Name.c_str();
				ImVec2 textSize = ImGui::CalcTextSize(filename);
				ImVec2 cursorPos = ImGui::GetCursorPos();
				
				ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
				ImGui::Button(("##" + std::to_string(i)).c_str(), { m_ThumbnailSize + padding * 2, m_ThumbnailSize + textSize.y + padding * 8 });
				if (ImGui::BeginDragDropSource())
				{
					eastl::string itemPath = file.DirectoryEntry.path().string().c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), (strlen(itemPath.c_str()) + 1) * sizeof(char));
					ImGui::EndDragDropSource();
				}
				ImGui::PopStyleColor();
				
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (file.DirectoryEntry.is_directory())
					{
						directoryToOpen = m_CurrentDirectory / file.DirectoryEntry.path().filename();
						directoryOpened = true;
					}
				}

				ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
				ImGui::SetItemAllowOverlap();
				ImGui::Image((ImTextureID)m_WhiteTexture->GetRendererID(), { m_ThumbnailSize, m_ThumbnailSize + textSize.y + overlayPaddingY }, { 0, 0 }, { 1, 1 }, EditorTheme::WindowBgAlternativeColor);

				ImDrawList& windowDrawList = *ImGui::GetWindowDrawList();

				ImVec2 rectMin = ImGui::GetItemRectMin();
				ImVec2 rectSize = ImGui::GetItemRectSize();

				ImGui::SetCursorPos({ cursorPos.x + thumbnailPadding * 0.75f, cursorPos.y + thumbnailPadding });
				ImGui::SetItemAllowOverlap();
				if (fontIcon)
				{
					/*
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
					ImGui::Button(fontIcon, { thumbnailSize, thumbnailSize });
					ImGui::PopStyleColor(3);
					*/
					ImGui::InvisibleButton(fontIcon, { thumbnailSize, thumbnailSize });
					windowDrawList.AddText({ rectMin.x + rectSize.x * 0.5f - ImGui::CalcTextSize(fontIcon).x * 0.5f, rectMin.y + rectSize.y * 0.5f}, ImColor(1.0f, 1.0f, 1.0f), fontIcon);
				}
				else
				{
					ImGui::Image((ImTextureID)textureId, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
				}

				rectMin = ImGui::GetItemRectMin();
				rectSize = ImGui::GetItemRectSize();

				if (textSize.x + padding * 2 <= rectSize.x)
				{
					float rectMin_x = rectMin.x - padding + (rectSize.x - textSize.x) / 2;
					float rectMin_y = rectMin.y + rectSize.y;

					float rectMax_x = rectMin_x + textSize.x + padding * 2;
					float rectMax_y = rectMin_y + textSize.y + padding * 2;

					windowDrawList.AddText({ rectMin_x + padding, rectMin_y + padding * 2 }, ImColor(1.0f, 1.0f, 1.0f), filename);
				}
				else
				{
					float rectMin_y = rectMin.y + rectSize.y;

					float rectMax_x = rectMin.x + rectSize.x;
					float rectMax_y = rectMin_y + textSize.y + padding * 2;

					ImGui::RenderTextEllipsis(&windowDrawList, { rectMin.x + padding, rectMin_y + padding * 2 }, { rectMax_x, rectMax_y }, rectMax_x, rectMax_x, filename, nullptr, &textSize);
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textSize.y + padding * 4);

				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		if (directoryOpened)
			UpdateDirectoryEntries(directoryToOpen);
	}

	void AssetPanel::UpdateDirectoryEntries(std::filesystem::path directory)
	{
		ARC_PROFILE_SCOPE();

		m_CurrentDirectory = directory;
		m_DirectoryEntries.clear();
		for (auto& directoryEntry : std::filesystem::directory_iterator(directory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			eastl::string fileNameString = relativePath.filename().string().c_str();
			m_DirectoryEntries.push_back({ fileNameString, directoryEntry });
		}
	}
}
