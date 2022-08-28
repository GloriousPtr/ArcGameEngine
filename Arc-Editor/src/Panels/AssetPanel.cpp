#include "AssetPanel.h"

#include <ArcEngine.h>
#include <Arc/Scene/EntitySerializer.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <filesystem>

#include "../EditorLayer.h"
#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	static const std::filesystem::path s_AssetPath = "Assets";

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
	
	static bool DragDropTarget(const char* dropPath)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
			{
				Entity entity = *((Entity*)payload->Data);
				std::filesystem::path path(dropPath);
				path /= eastl::string(entity.GetComponent<TagComponent>().Tag + ".prefab").c_str();
				EntitySerializer::SerializeEntityAsPrefab(path.string().c_str(), entity);
				return true;
			}

			ImGui::EndDragDropTarget();
		}

		return false;
	}

	static void DragDropFrom(const char* filepath)
	{
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", filepath, (strlen(filepath) + 1) * sizeof(char));
			ImGui::Text(StringUtils::GetName(filepath).c_str());
			ImGui::EndDragDropSource();
		}
	}

	std::pair<bool, uint32_t> AssetPanel::DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selectionMask, ImGuiTreeNodeFlags flags)
	{
		ARC_PROFILE_SCOPE();

		bool anyNodeClicked = false;
		uint32_t nodeClicked = 0;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			ImGuiTreeNodeFlags nodeFlags = flags;

			auto& entryPath = entry.path();

			bool entryIsFile = !std::filesystem::is_directory(entryPath);
			if (entryIsFile)
				nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			const bool selected = (*selectionMask & BIT(*count)) != 0;
			if (selected)
			{
				nodeFlags |= ImGuiTreeNodeFlags_Selected;
				ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::HeaderSelectedColor);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderSelectedColor);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderHoveredColor);
			}

			bool open = ImGui::TreeNodeEx((void*)(intptr_t)(*count), nodeFlags, "");
			ImGui::PopStyleColor(selected ? 2 : 1);

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				if (!entryIsFile)
					UpdateDirectoryEntries(entryPath);

				nodeClicked = *count;
				anyNodeClicked = true;
			}

			const std::string filepath = entryPath.string();

			if (!entryIsFile)
				DragDropTarget(filepath.c_str());
			DragDropFrom(filepath.c_str());

			eastl::string name = StringUtils::GetNameWithExtension(filepath.c_str());
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
					auto clickState = DirectoryTreeViewRecursive(entryPath, count, selectionMask, flags);

					if (!anyNodeClicked)
					{
						anyNodeClicked = clickState.first;
						nodeClicked = clickState.second;
					}

					ImGui::TreePop();
				}
				else
				{
					for (const auto& e : std::filesystem::recursive_directory_iterator(entryPath))
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

		m_CurrentDirectory = s_AssetPath;
		UpdateDirectoryEntries(s_AssetPath);
	}

	void AssetPanel::OnUpdate(Timestep ts)
	{
		m_ElapsedTime += ts;
	}

	void AssetPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoScrollbar;

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable
			| ImGuiTableFlags_ContextMenuInBody;

		if (OnBegin(windowFlags))
		{
			RenderHeader();
			ImGui::Separator();
			ImVec2 availableRegion = ImGui::GetContentRegionAvail();
			if (ImGui::BeginTable("MainViewTable", 2, tableFlags, availableRegion))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				RenderSideView();
				ImGui::TableNextColumn();
				RenderBody(m_ThumbnailSize >= 64.0f);

				ImGui::EndTable();
			}
			OnEnd();
		}
	}

	void AssetPanel::RenderHeader()
	{
		ARC_PROFILE_SCOPE();

		if (ImGui::Button(ICON_MDI_COGS))
			ImGui::OpenPopup("SettingsPopup");
		if (ImGui::BeginPopup("SettingsPopup"))
		{
			UI::BeginProperties(ImGuiTableFlags_SizingStretchSame);
			UI::Property("Thumbnail Size", m_ThumbnailSize, 63.9f, 128.0f, nullptr, "");
			UI::EndProperties();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		const float cursorPosX = ImGui::GetCursorPosX();
		m_Filter.Draw("###ConsoleFilter", ImGui::GetContentRegionAvail().x);
		if(!m_Filter.IsActive())
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(cursorPosX + ImGui::GetFontSize() * 0.5f);
			ImGui::TextUnformatted(ICON_MDI_MAGNIFY " Search...");
		}

		ImGui::Spacing();
		ImGui::Spacing();

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

			if (ImGui::Button(ICON_MDI_ARROW_LEFT_CIRCLE_OUTLINE))
			{
				m_BackStack.push(m_CurrentDirectory);
				UpdateDirectoryEntries(m_CurrentDirectory.parent_path());
			}

			if (disabledBackButton)
			{
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}
		}

		ImGui::SameLine();

		// Front button
		{
			bool disabledFrontButton = false;
			if (m_BackStack.empty())
				disabledFrontButton = true;

			if (disabledFrontButton)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::Button(ICON_MDI_ARROW_RIGHT_CIRCLE_OUTLINE))
			{
				auto& top = m_BackStack.top();
				UpdateDirectoryEntries(top);
				m_BackStack.pop();
			}

			if (disabledFrontButton)
			{
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}
		}

		ImGui::SameLine();

		ImGui::Text(ICON_MDI_FOLDER);

		std::filesystem::path current = "";
		std::filesystem::path directoryToOpen = "";
		for (auto& path : m_CurrentDirectory)
		{
			current /= path;

			ImGui::SameLine();

			const char* folderName = StringUtils::GetName(path.string().c_str()).c_str();
			ImGui::Text("%s %s", folderName, ICON_MDI_GREATER_THAN);

			if (ImGui::IsItemClicked() && ImGui::IsMouseDown(0))
				directoryToOpen = current;
		}

		if (!directoryToOpen.empty())
			UpdateDirectoryEntries(directoryToOpen);
	}

	void AssetPanel::RenderSideView()
	{
		ARC_PROFILE_SCOPE();

		static int selectionMask = 0;

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg
			| ImGuiTableFlags_NoPadInnerX
			| ImGuiTableFlags_NoPadOuterX
			| ImGuiTableFlags_ContextMenuInBody
			| ImGuiTableFlags_ScrollY;

		constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_FramePadding
			| ImGuiTreeNodeFlags_SpanFullWidth;
		
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
		if (ImGui::BeginTable("SideViewTable", 1, tableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGuiTreeNodeFlags nodeFlags = treeNodeFlags;
			const bool selected = m_CurrentDirectory == s_AssetPath && selectionMask == 0;
			if (selected)
			{
				nodeFlags |= ImGuiTreeNodeFlags_Selected;
				ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::HeaderSelectedColor);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderSelectedColor);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderHoveredColor);
			}

			bool opened = ImGui::TreeNodeEx(s_AssetPath.c_str(), nodeFlags, "");
			ImGui::PopStyleColor(selected ? 2 : 1);

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

				auto clickState = DirectoryTreeViewRecursive(s_AssetPath, &count, &selectionMask, treeNodeFlags);

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

			ImGui::EndTable();

			if (ImGui::IsItemClicked())
				EditorLayer::GetInstance()->SetContext(EditorContextType::None, 0, 0);
		}

		ImGui::PopStyleVar();
	}

	void AssetPanel::RenderBody(bool grid)
	{
		ARC_PROFILE_SCOPE();

		std::filesystem::path directoryToOpen = "";

		float padding = 4.0f;
		float cellSize = m_ThumbnailSize + 2 * padding;

		float overlayPaddingY = 6.0f * padding;
		float thumbnailPadding = overlayPaddingY * 0.5f;
		float thumbnailSize = m_ThumbnailSize - thumbnailPadding;

		float panelWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ScrollbarSize;
		int columnCount = (int) (panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		float lineHeight = ImGui::GetTextLineHeight();
		ImVec2 framePadding = ImGui::GetStyle().FramePadding;

		uint32_t flags = ImGuiTableFlags_ContextMenuInBody
			| ImGuiTableFlags_NoPadInnerX
			| ImGuiTableFlags_ScrollY;

		uint32_t i = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });

		if (!grid)
		{
			columnCount = 1;
			flags |= ImGuiTableFlags_RowBg | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_SizingStretchSame;
		}
		else
		{
			flags |= ImGuiTableFlags_PadOuterX | ImGuiTableFlags_SizingFixedFit;
		}

		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImVec2 region = ImGui::GetContentRegionAvail();
		ImGui::InvisibleButton("##DragDropTargetAssetPanelBody", region);
		
		if (DragDropTarget(m_CurrentDirectory.string().c_str()))
			UpdateDirectoryEntries(m_CurrentDirectory);
		ImGui::SetItemAllowOverlap();
		ImGui::SetCursorPos(cursorPos);

		bool anyItemHovered = false;

		if (ImGui::BeginTable("BodyTable", columnCount, flags))
		{
			for (auto& file : m_DirectoryEntries)
			{
				ImGui::PushID(i++);

				bool isDir = file.DirectoryEntry.is_directory();
				std::string filepath = file.DirectoryEntry.path().string();
				const char* filename = file.Name.c_str();

				uint32_t textureId = m_DirectoryIcon->GetRendererID();
				const char* fontIcon = nullptr;
				if (!isDir)
				{
					eastl::string ext = StringUtils::GetExtension((eastl::string&&)file.Name);
					if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
						textureId = AssetManager::GetTexture2D(filepath.c_str())->GetRendererID();
					else
						fontIcon = GetFileIcon(ext.c_str());
				}

				ImGui::TableNextColumn();

				if (grid)
				{
					ImVec2 textSize = ImGui::CalcTextSize(filename);
					ImVec2 cursorPos = ImGui::GetCursorPos();

					bool highlight = false;
					EditorContext context = EditorLayer::GetInstance()->GetContext();
					if (context.Type == EditorContextType::File && (char*)context.Data)
					{
						const char* path = (char*)context.Data;
						highlight = path == file.DirectoryEntry.path();
					}

					bool clicked = UI::ToggleButton(("##" + std::to_string(i)).c_str(), highlight, { m_ThumbnailSize + padding * 2, m_ThumbnailSize + textSize.y + padding * 8 }, 0.0f, 1.0f);
					if (m_ElapsedTime > 0.25f && clicked)
					{
						auto& path = m_CurrentDirectory / file.DirectoryEntry.path().filename();
						std::string strPath = path.string();
						EditorLayer::GetInstance()->SetContext(EditorContextType::File, (void*)strPath.c_str(), sizeof(char) * (strlen(strPath.c_str()) + 1));
					}
					if (isDir)
						DragDropTarget(filepath.c_str());

					DragDropFrom(filepath.c_str());

					if (ImGui::IsItemHovered())
						anyItemHovered = true;

					if (isDir && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					{
						directoryToOpen = m_CurrentDirectory / file.DirectoryEntry.path().filename();
						EditorLayer::GetInstance()->SetContext(EditorContextType::None, 0, 0);
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
						ImGui::InvisibleButton(fontIcon, { thumbnailSize, thumbnailSize });
						windowDrawList.AddText({ rectMin.x + rectSize.x * 0.5f - ImGui::CalcTextSize(fontIcon).x * 0.5f, rectMin.y + rectSize.y * 0.5f }, ImColor(1.0f, 1.0f, 1.0f), fontIcon);
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
				}
				else
				{
					constexpr ImGuiTreeNodeFlags teeNodeFlags = ImGuiTreeNodeFlags_FramePadding
						| ImGuiTreeNodeFlags_SpanFullWidth
						| ImGuiTreeNodeFlags_Leaf;

					bool opened = ImGui::TreeNodeEx(file.Name.c_str(), teeNodeFlags, "");

					if (ImGui::IsItemHovered())
						anyItemHovered = true;

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && file.DirectoryEntry.is_directory())
						directoryToOpen = m_CurrentDirectory / file.DirectoryEntry.path().filename();

					DragDropFrom(file.DirectoryEntry.path().string().c_str());

					ImGui::SameLine();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - lineHeight);
					if (fontIcon)
						ImGui::TextUnformatted(fontIcon);
					else
						ImGui::Image((ImTextureID)textureId, { lineHeight, lineHeight }, { 0, 1 }, { 1, 0 });
					ImGui::SameLine();
					ImGui::TextUnformatted(filename);

					if (opened)
						ImGui::TreePop();
				}

				ImGui::PopID();
			}

			ImGui::EndTable();

			if (!anyItemHovered && ImGui::IsItemClicked())
				EditorLayer::GetInstance()->SetContext(EditorContextType::None, 0, 0);
		}
		ImGui::PopStyleVar();

		if (!directoryToOpen.empty())
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

		m_ElapsedTime = 0.0f;
	}
}
