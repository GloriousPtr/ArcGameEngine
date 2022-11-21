#include "AssetPanel.h"

#include <ArcEngine.h>
#include <Arc/Scene/EntitySerializer.h>

#include <icons/IconsMaterialDesignIcons.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <filesystem>

#include "../EditorLayer.h"
#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	static const eastl::hash_map<eastl::string, eastl::string> s_FileTypes =
	{
		{ "png",	"Texture" },
		{ "jpg",	"Texture" },
		{ "jpeg",	"Texture" },
		{ "bmp",	"Texture" },
		{ "gif",	"Texture" },

		{ "hdr",	"Cubemap" },
		{ "tga",	"Cubemap" },

		{ "glsl",	"Shader" },

		{ "obj",	"Model" },
		{ "fbx",	"Model" },
		{ "gltf",	"Model" },
		{ "assbin",	"Model" },

		{ "mp3",	"Audio" },
		{ "m4a",	"Audio" },
		{ "wav",	"Audio" },

		{ "arc",	"Scene" },
		{ "prefab", "Prefab" },
	};

	static const eastl::hash_map<eastl::string, ImVec4> s_TypeColors =
	{
		{ "Texture",		{ 0.80f, 0.20f, 0.30f, 1.00f } },
		{ "Cubemap",		{ 0.80f, 0.20f, 0.30f, 1.00f } },
		{ "Shader",			{ 0.20f, 0.40f, 0.75f, 1.00f } },
		{ "Model",			{ 0.20f, 0.80f, 0.75f, 1.00f } },
		{ "Audio",			{ 0.20f, 0.80f, 0.50f, 1.00f } },

		{ "Scene",			{ 0.75f, 0.35f, 0.20f, 1.00f } },
		{ "Prefab",			{ 0.10f, 0.50f, 0.80f, 1.00f } },
	};

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

	static void DragDropFrom(const eastl::string& filepath)
	{
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", filepath.c_str(), filepath.length() + 1);
			ImGui::Text(StringUtils::GetName(eastl::move(filepath)).c_str());
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

			const eastl::string filepath = entryPath.string().c_str();

			if (!entryIsFile)
				DragDropTarget(filepath.c_str());
			DragDropFrom(filepath);

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
			ImGui::TextUnformatted(name.c_str());
			m_CurrentlyVisibleItemsTreeView++;

			(*count)--;

			if (!entryIsFile)
			{
				if (open)
				{
					const auto [isClicked, clickedNode] = DirectoryTreeViewRecursive(entryPath, count, selectionMask, flags);

					if (!anyNodeClicked)
					{
						anyNodeClicked = isClicked;
						nodeClicked = clickedNode;
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
		: BasePanel(name, ICON_MDI_FOLDER_STAR, true)
	{
		ARC_PROFILE_SCOPE();

		m_WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		m_WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");

		m_AssetsDirectory = Project::GetAssetDirectory();
		m_CurrentDirectory = m_AssetsDirectory;
		UpdateDirectoryEntries(m_CurrentDirectory);
	}

	void AssetPanel::OnUpdate([[maybe_unused]] Timestep ts)
	{
		ARC_PROFILE_SCOPE();

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
				RenderBody(m_ThumbnailSize >= 96.0f);

				ImGui::EndTable();
			}
			OnEnd();
		}
	}

	void AssetPanel::Invalidate()
	{
		m_AssetsDirectory = Project::GetAssetDirectory();
		m_CurrentDirectory = m_AssetsDirectory;
		UpdateDirectoryEntries(m_CurrentDirectory);
	}

	void AssetPanel::RenderHeader()
	{
		ARC_PROFILE_SCOPE();

		if (ImGui::Button(ICON_MDI_COGS))
			ImGui::OpenPopup("SettingsPopup");
		if (ImGui::BeginPopup("SettingsPopup"))
		{
			UI::BeginProperties(ImGuiTableFlags_SizingStretchSame);
			UI::Property("Thumbnail Size", m_ThumbnailSize, 95.9f, 256.0f, nullptr, 0.1f, "");
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
			if (m_CurrentDirectory == m_AssetsDirectory)
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
				const auto& top = m_BackStack.top();
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

		ImGui::TextUnformatted(ICON_MDI_FOLDER);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 0.0f });
		std::filesystem::path current = "";
		std::filesystem::path directoryToOpen = "";
		for (auto& path : m_CurrentDirectory)
		{
			current /= path;
			ImGui::SameLine();
			eastl::string folderName = StringUtils::GetName(path.string().c_str());
			if (folderName.empty())
				folderName = "Assets";
			if (ImGui::Button(folderName.c_str()))
				directoryToOpen = current;

			if (m_CurrentDirectory != current)
			{
				ImGui::SameLine();
				ImGui::TextUnformatted("/");
			}
		}
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();

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
			const bool selected = m_CurrentDirectory == m_AssetsDirectory && selectionMask == 0;
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

			bool opened = ImGui::TreeNodeEx(m_AssetsDirectory.string().c_str(), nodeFlags, "");
			ImGui::PopStyleColor(selected ? 2 : 1);

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				UpdateDirectoryEntries(m_AssetsDirectory);
				selectionMask = 0;
			}
			const char* folderIcon = opened ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
			ImGui::TextUnformatted(folderIcon);
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::TextUnformatted("Assets");

			if (opened)
			{
				uint32_t count = 0;
				for (const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetsDirectory))
					count++;

				const auto [isClicked, clickedNode] = DirectoryTreeViewRecursive(m_AssetsDirectory, &count, &selectionMask, treeNodeFlags);

				if (isClicked)
				{
					// (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
					if (ImGui::GetIO().KeyCtrl)
						selectionMask ^= BIT(clickedNode);          // CTRL+click to toggle
					else
						selectionMask = BIT(clickedNode);           // Click to single-select
				}

				ImGui::TreePop();
			}

			ImGui::EndTable();

			if (ImGui::IsItemClicked())
				EditorLayer::GetInstance()->ResetContext();
		}

		ImGui::PopStyleVar();
	}

	void AssetPanel::RenderBody(bool grid)
	{
		ARC_PROFILE_SCOPE();

		std::filesystem::path directoryToOpen = "";

		float padding = 4.0f;
		float scaledThumbnailSize = m_ThumbnailSize * ImGui::GetIO().FontGlobalScale;
		float scaledThumbnailSizeX = scaledThumbnailSize * 0.55f;
		float cellSize = scaledThumbnailSizeX + 2 * padding + scaledThumbnailSizeX * 0.1f;

		float overlayPaddingY = 6.0f * padding;
		float thumbnailPadding = overlayPaddingY * 0.5f;
		float thumbnailSize = scaledThumbnailSizeX - thumbnailPadding;

		ImVec2 backgroundThumbnailSize = { scaledThumbnailSizeX + padding * 2, scaledThumbnailSize + padding * 2 };

		float panelWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ScrollbarSize;
		int columnCount = (int) (panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		float lineHeight = ImGui::GetTextLineHeight();
		uint32_t flags = ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_ScrollY;

		if (!grid)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
			columnCount = 1;
			flags |= ImGuiTableFlags_RowBg
					| ImGuiTableFlags_NoPadOuterX
					| ImGuiTableFlags_NoPadInnerX
					| ImGuiTableFlags_SizingStretchSame;
		}
		else
		{
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { scaledThumbnailSizeX * 0.05f, scaledThumbnailSizeX * 0.05f });
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
		bool textureCreated = false;
		if (ImGui::BeginTable("BodyTable", columnCount, flags))
		{
			for (int i = 0, len = (int)m_DirectoryEntries.size(); i < len; ++i)
			{
				ImGui::PushID(i);

				auto& file = m_DirectoryEntries[i];

				bool isDir = file.IsDirectory;
				const char* filename = file.Name.c_str();

				uint64_t textureId = m_DirectoryIcon->GetRendererID();
				if (!isDir)
				{
					eastl::string ext = StringUtils::GetExtension((eastl::string&&)file.Name);
					if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
					{
						if (file.Thumbnail)
						{
							textureId = file.Thumbnail->GetRendererID();
						}
						else if (!textureCreated)
						{
							textureCreated = true;
							file.Thumbnail = AssetManager::GetTexture2D(file.Filepath);
							textureId = file.Thumbnail->GetRendererID();
						}
						else
						{
							textureId = 0;
						}
					}
					else
					{
						textureId = m_FileIcon->GetRendererID();
					}
				}

				ImGui::TableNextColumn();

				if (grid)
				{
					cursorPos = ImGui::GetCursorPos();

					bool highlight = false;
					const EditorContext& context = EditorLayer::GetInstance()->GetContext();
					if (context.IsValid(EditorContextType::File))
					{
						highlight = file.Filepath == context.Data;
					}

					// Background button
					static eastl::string id = "###";
					id[2] = (char)i;
					bool const clicked = UI::ToggleButton(id.c_str(), highlight, backgroundThumbnailSize, 0.0f, 1.0f);
					if (m_ElapsedTime > 0.25f && clicked)
					{
						const auto& path = m_CurrentDirectory / file.DirectoryEntry.path().filename();
						std::string strPath = path.string();
						EditorLayer::GetInstance()->SetContext(EditorContextType::File, (void*)strPath.c_str(), sizeof(char) * (strPath.length() + 1));
					}
					if (isDir)
						DragDropTarget(file.Filepath.c_str());

					DragDropFrom(file.Filepath);

					if (ImGui::IsItemHovered())
						anyItemHovered = true;

					if (isDir && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					{
						directoryToOpen = m_CurrentDirectory / file.DirectoryEntry.path().filename();
						EditorLayer::GetInstance()->ResetContext();
					}

					// Forground Image
					ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
					ImGui::SetItemAllowOverlap();
					ImGui::Image((ImTextureID)m_WhiteTexture->GetRendererID(), { backgroundThumbnailSize.x - padding * 2.0f, backgroundThumbnailSize.y - padding * 2.0f }, { 0, 0 }, { 1, 1 }, EditorTheme::WindowBgAlternativeColor);

					// Thumbnail Image
					ImGui::SetCursorPos({ cursorPos.x + thumbnailPadding * 0.75f, cursorPos.y + thumbnailPadding });
					ImGui::SetItemAllowOverlap();
					ImGui::Image((ImTextureID)textureId, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

					// Type Color frame
					const char* fileType = "Unknown";
					if (s_FileTypes.find_as(file.Extension.c_str()) != s_FileTypes.end())
						fileType = s_FileTypes.at(file.Extension.c_str()).c_str();
					ImVec4 typeColor = { 1.0f, 1.0f, 1.0f, 1.0f };
					if (s_TypeColors.find_as(fileType) != s_TypeColors.end())
						typeColor = s_TypeColors.at(fileType);
					ImVec2 typeColorFrameSize = { scaledThumbnailSizeX, scaledThumbnailSizeX * 0.03f };
					ImGui::SetCursorPosX(cursorPos.x + padding);
					ImGui::Image((ImTextureID)m_WhiteTexture->GetRendererID(), typeColorFrameSize, { 0, 0 }, { 1, 1 }, isDir ? ImVec4(0.0f, 0.0f, 0.0f, 0.0f) : typeColor);

					ImVec2 rectMin = ImGui::GetItemRectMin();
					ImVec2 rectSize = ImGui::GetItemRectSize();
					ImRect clipRect = ImRect({ rectMin.x + padding * 1.0f, rectMin.y + padding * 2.0f },
						{ rectMin.x + rectSize.x, rectMin.y + scaledThumbnailSizeX - EditorTheme::SmallFont->FontSize - padding * 4.0f });
					UI::ClippedText(clipRect.Min, clipRect.Max, filename, nullptr, nullptr, { 0, 0 }, &clipRect, scaledThumbnailSizeX - padding * 2.0f);
					
					if (!isDir)
					{
						ImGui::SetCursorPos({ cursorPos.x + padding * 2.0f, cursorPos.y + backgroundThumbnailSize.y - EditorTheme::SmallFont->FontSize - padding * 2.0f });
						ImGui::BeginDisabled();
						ImGui::PushFont(EditorTheme::SmallFont);
						ImGui::TextUnformatted(fileType);
						ImGui::PopFont();
						ImGui::EndDisabled();
					}
				}
				else
				{
					constexpr ImGuiTreeNodeFlags teeNodeFlags = ImGuiTreeNodeFlags_FramePadding
						| ImGuiTreeNodeFlags_SpanFullWidth
						| ImGuiTreeNodeFlags_Leaf;

					bool opened = ImGui::TreeNodeEx(file.Name.c_str(), teeNodeFlags, "");

					if (ImGui::IsItemHovered())
						anyItemHovered = true;

					if (isDir && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						directoryToOpen = m_CurrentDirectory / file.DirectoryEntry.path().filename();

					DragDropFrom(file.Filepath.c_str());

					ImGui::SameLine();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - lineHeight);
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
				EditorLayer::GetInstance()->ResetContext();
		}

		ImGui::PopStyleVar();

		if (!directoryToOpen.empty())
			UpdateDirectoryEntries(directoryToOpen);
	}

	void AssetPanel::UpdateDirectoryEntries(const std::filesystem::path& directory)
	{
		ARC_PROFILE_SCOPE();

		m_CurrentDirectory = directory;
		m_DirectoryEntries.clear();

		for (auto& directoryEntry : std::filesystem::directory_iterator(directory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, m_AssetsDirectory);
			eastl::string filename = relativePath.filename().string().c_str();
			m_DirectoryEntries.emplace_back(filename, path.string().c_str(), StringUtils::GetExtension(filename.c_str()), directoryEntry, nullptr, directoryEntry.is_directory());
		}

		m_ElapsedTime = 0.0f;
	}
}
