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

	static const char* GetFileIcon(std::string& filename)
	{
		auto lastDot = filename.find_last_of(".");
		std::string name = filename.substr(lastDot + 1, filename.size() - lastDot);

		if (name == "txt" || name == "md")
			return ICON_MDI_FILE_DOCUMENT;
		if (name == "png" || name == "jpg" || name == "jpeg" || name == "bmp" || name == "gif")
			return ICON_MDI_FILE_IMAGE;
		if (name == "hdr" || name == "tga")
			return ICON_MDI_IMAGE_FILTER_HDR;
		if (name == "glsl")
			return ICON_MDI_IMAGE_FILTER_BLACK_WHITE;
		if (name == "obj" || name == "fbx" || name == "gltf")
			return ICON_MDI_VECTOR_POLYGON;

		return ICON_MDI_FILE;
	}
	
	std::pair<bool, uint32_t> AssetPanel::DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selectionMask)
	{
		OPTICK_EVENT();

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		bool anyNodeClicked = false;
		uint32_t nodeClicked = 0;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			ImGuiTreeNodeFlags nodeFlags = flags;

			const bool selected = (*selectionMask & BIT(*count)) != 0;
			if (selected)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			std::string name = entry.path().string();

			auto lastSlash = name.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
			name = name.substr(lastSlash, name.size() - lastSlash);

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
				folderIcon = GetFileIcon(name);
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
		OPTICK_EVENT();

		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");

		UpdateDirectoryEntries(s_AssetPath);
	}

	void AssetPanel::OnImGuiRender()
	{
		OPTICK_EVENT();

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
		OPTICK_EVENT();

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
		OPTICK_EVENT();

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
		OPTICK_EVENT();

		std::filesystem::path directoryToOpen;
		bool directoryOpened = false;

		float padding = 5.0f;
		float cellSize = m_ThumbnailSize + 2 * padding;

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
				if (!file.DirectoryEntry.is_directory())
				{
					auto lastDot = file.Name.find_last_of(".");
					std::string ext = file.Name.substr(lastDot + 1, file.Name.size() - lastDot);
					if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
					{
						std::string path = file.DirectoryEntry.path().string();
						textureId = AssetManager::GetTexture2D(path.c_str())->GetRendererID();
					}
				}

				ImGui::TableNextColumn();
				ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
				ImGui::ImageButton((ImTextureID)textureId, { m_ThumbnailSize, m_ThumbnailSize }, { 0, 1 }, { 1, 0 }, padding);
				
				if (ImGui::BeginDragDropSource())
				{
					std::string itemPath = file.DirectoryEntry.path().string();
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

				float posX = ImGui::GetCursorPosX() + cellSize * 0.5f;
				const char* fileName = file.Name.c_str();
				ImGui::SetCursorPosX(posX - ImGui::CalcTextSize(fileName).x * 0.5f);
				ImGui::TextUnformatted(fileName);

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
		OPTICK_EVENT();

		m_CurrentDirectory = directory;
		m_DirectoryEntries.clear();
		for (auto& directoryEntry : std::filesystem::directory_iterator(directory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			std::string fileNameString = relativePath.filename().string();
			m_DirectoryEntries.push_back({ fileNameString, directoryEntry });
		}
	}
}
