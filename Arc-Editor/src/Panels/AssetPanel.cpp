#include "AssetPanel.h"

#include <Arc/Scene/EntitySerializer.h>
#include <Arc/Core/Filesystem.h>
#include <Platform/VisualStudio/VisualStudioAccessor.h>

#include <icons/IconsMaterialDesignIcons.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include "../EditorLayer.h"
#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"


namespace ArcEngine
{
	static const std::unordered_map<FileType, const char*> s_FileTypesToString =
	{
		{ FileType::Unknown,	"Unknown" },

		{ FileType::Scene,		"Scene" },
		{ FileType::Prefab,		"Prefab" },
		{ FileType::Script,		"Script" },
		{ FileType::Shader,		"Shader" },

		{ FileType::Texture,	"Texture" },
		{ FileType::Cubemap,	"Cubemap" },
		{ FileType::Model,		"Model" },

		{ FileType::Audio,		"Audio" },
	};

	static const std::unordered_map<std::string, FileType, UM_StringTransparentEquality> s_FileTypes =
	{
		{ ".arc",		FileType::Scene },
		{ ".prefab",	FileType::Prefab },
		{ ".cs",		FileType::Script },
		{ ".glsl",		FileType::Shader },

		{ ".png",		FileType::Texture },
		{ ".jpg",		FileType::Texture },
		{ ".jpeg",		FileType::Texture },
		{ ".bmp",		FileType::Texture },
		{ ".gif",		FileType::Texture },

		{ ".hdr",		FileType::Cubemap },
		{ ".tga",		FileType::Cubemap },

		{ ".obj",		FileType::Model },
		{ ".fbx",		FileType::Model },
		{ ".gltf",		FileType::Model },

		{ ".mp3",		FileType::Audio },
		{ ".m4a",		FileType::Audio },
		{ ".wav",		FileType::Audio },
	};

	static const std::unordered_map<FileType, ImVec4> s_TypeColors =
	{
		{ FileType::Scene,			{ 0.75f, 0.35f, 0.20f, 1.00f } },
		{ FileType::Prefab,			{ 0.10f, 0.50f, 0.80f, 1.00f } },
		{ FileType::Script,			{ 0.10f, 0.50f, 0.80f, 1.00f } },
		{ FileType::Shader,			{ 0.10f, 0.50f, 0.80f, 1.00f } },

		{ FileType::Texture,		{ 0.80f, 0.20f, 0.30f, 1.00f } },
		{ FileType::Cubemap,		{ 0.80f, 0.20f, 0.30f, 1.00f } },
		{ FileType::Model,			{ 0.20f, 0.80f, 0.75f, 1.00f } },

		{ FileType::Audio,			{ 0.20f, 0.80f, 0.50f, 1.00f } },
	};

	static const char8_t* GetFileIcon(const char* ext)
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
	
	static bool DragDropTarget(const std::filesystem::path& dropPath)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
			{
				Entity entity = *((Entity*)payload->Data);
				std::filesystem::path path = dropPath / std::string(entity.GetComponent<TagComponent>().Tag + ".prefab");
				EntitySerializer::SerializeEntityAsPrefab(path.string().c_str(), entity);
				return true;
			}

			ImGui::EndDragDropTarget();
		}

		return false;
	}

	static void DragDropFrom(const std::filesystem::path& filepath)
	{
		if (ImGui::BeginDragDropSource())
		{
			std::string pathStr = filepath.string();
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", pathStr.c_str(), pathStr.length() + 1);
			ImGui::TextUnformatted(filepath.filename().string().c_str());
			ImGui::EndDragDropSource();
		}
	}

	static void OpenFile(const std::filesystem::path& path)
	{
		std::string filepathString = path.string();
		const char* filepath = filepathString.c_str();
		std::string ext = path.extension().string();
		const auto& fileTypeIt = s_FileTypes.find(ext);
		if (fileTypeIt != s_FileTypes.end())
		{
			FileType fileType = fileTypeIt->second;
			switch (fileType)
			{
				case FileType::Scene:
					EditorLayer::GetInstance()->OpenScene(filepath);
					break;
				case FileType::Script:
				case FileType::Shader:
					VisualStudioAccessor::OpenFile(filepath);
					break;
				case FileType::Unknown:
				case FileType::Prefab:
				case FileType::Texture:
				case FileType::Cubemap:
				case FileType::Model:
				case FileType::Audio:
				default:
					FileDialogs::OpenFileWithProgram(filepath);
			}
		}
		else
		{
			FileDialogs::OpenFileWithProgram(filepath);
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
				DragDropTarget(entryPath);
			DragDropFrom(entryPath);

			std::string name = StringUtils::GetNameWithExtension(filepath);
			const char8_t* folderIcon;
			if (entryIsFile)
				folderIcon = GetFileIcon(entryPath.extension().string().c_str());
			else
				folderIcon = open ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
			
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
			ImGui::TextUnformatted((const char*)folderIcon);
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
		Refresh();
	}

	void AssetPanel::OnUpdate([[maybe_unused]] Timestep ts)
	{
		ARC_PROFILE_SCOPE();

		m_ElapsedTime += ts;

		if (ImGui::IsKeyPressed(ImGuiKey_Delete) && GImGui->ActiveId == 0)
		{
			const EditorContext& context = EditorLayer::GetInstance()->GetContext();
			if (context.IsValid(EditorContextType::File))
			{
				const std::filesystem::path path = context.As<char>();
				std::filesystem::remove_all(path);
				EditorLayer::GetInstance()->ResetContext();
				Refresh();
			}
		}
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
		Refresh();
	}

	void AssetPanel::RenderHeader()
	{
		ARC_PROFILE_SCOPE();

		if (ImGui::Button((const char*)ICON_MDI_COGS))
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
			ImGui::TextUnformatted((const char*)ICON_MDI_MAGNIFY " Search...");
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

			if (ImGui::Button((const char*)ICON_MDI_ARROW_LEFT_CIRCLE_OUTLINE))
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

			if (ImGui::Button((const char*)ICON_MDI_ARROW_RIGHT_CIRCLE_OUTLINE))
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

		ImGui::TextUnformatted((const char*)ICON_MDI_FOLDER);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 0.0f });
		std::filesystem::path current = m_AssetsDirectory.parent_path();
		std::filesystem::path directoryToOpen = "";
		std::filesystem::path currentDirectory = std::filesystem::relative(m_CurrentDirectory, current);
		for (auto& path : currentDirectory)
		{
			current /= path;
			ImGui::SameLine();
			if (ImGui::Button(path.filename().string().c_str()))
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
			UpdateDirectoryEntries(m_AssetsDirectory / directoryToOpen);
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
			const char8_t* folderIcon = opened ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
			ImGui::TextUnformatted((const char*)folderIcon);
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
		std::filesystem::path directoryToDelete = "";

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
			Refresh();
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
					if (file.Type == FileType::Texture)
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

				const auto& path = file.DirectoryEntry.path();
				std::string strPath = path.string();

				if (grid)
				{
					cursorPos = ImGui::GetCursorPos();

					bool highlight = false;
					const EditorContext& context = EditorLayer::GetInstance()->GetContext();
					if (context.IsValid(EditorContextType::File))
					{
						highlight = file.Filepath == context.As<char>();
					}

					// Background button
					static std::string id = "###";
					id[2] = (char)i;
					bool const clicked = UI::ToggleButton(id.c_str(), highlight, backgroundThumbnailSize, 0.0f, 1.0f);
					if (m_ElapsedTime > 0.25f && clicked)
					{
						EditorLayer::GetInstance()->SetContext(EditorContextType::File, (void*)strPath.c_str(), sizeof(char) * (strPath.length() + 1));
					}
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);
					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::MenuItem("Delete"))
						{
							directoryToDelete = path;
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::MenuItem("Rename"))
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::Separator();

						DrawContextMenuItems(path, isDir);
						ImGui::EndPopup();
					}
					ImGui::PopStyleVar();

					if (isDir)
						DragDropTarget(file.Filepath.c_str());

					DragDropFrom(file.Filepath);

					if (ImGui::IsItemHovered())
						anyItemHovered = true;

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					{
						if (isDir)
							directoryToOpen = path;
						else
							OpenFile(path);
						EditorLayer::GetInstance()->ResetContext();
					}

					// Foreground Image
					ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
					ImGui::SetItemAllowOverlap();
					ImGui::Image((ImTextureID)m_WhiteTexture->GetRendererID(), { backgroundThumbnailSize.x - padding * 2.0f, backgroundThumbnailSize.y - padding * 2.0f }, { 0, 0 }, { 1, 1 }, EditorTheme::WindowBgAlternativeColor);

					// Thumbnail Image
					ImGui::SetCursorPos({ cursorPos.x + thumbnailPadding * 0.75f, cursorPos.y + thumbnailPadding });
					ImGui::SetItemAllowOverlap();
					ImGui::Image((ImTextureID)textureId, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

					// Type Color frame
					

					ImVec2 typeColorFrameSize = { scaledThumbnailSizeX, scaledThumbnailSizeX * 0.03f };
					ImGui::SetCursorPosX(cursorPos.x + padding);
					ImGui::Image((ImTextureID)m_WhiteTexture->GetRendererID(), typeColorFrameSize, { 0, 0 }, { 1, 1 }, isDir ? ImVec4(0.0f, 0.0f, 0.0f, 0.0f) : file.FileTypeIndicatorColor);

					ImVec2 rectMin = ImGui::GetItemRectMin();
					ImVec2 rectSize = ImGui::GetItemRectSize();
					ImRect clipRect = ImRect({ rectMin.x + padding * 1.0f, rectMin.y + padding * 2.0f },
						{ rectMin.x + rectSize.x, rectMin.y + scaledThumbnailSizeX - EditorTheme::SmallFont->FontSize - padding * 4.0f });
					UI::ClippedText(clipRect.Min, clipRect.Max, filename, nullptr, nullptr, { 0, 0 }, nullptr, clipRect.GetSize().x);

					if (!isDir)
					{
						ImGui::SetCursorPos({ cursorPos.x + padding * 2.0f, cursorPos.y + backgroundThumbnailSize.y - EditorTheme::SmallFont->FontSize - padding * 2.0f });
						ImGui::BeginDisabled();
						ImGui::PushFont(EditorTheme::SmallFont);
						ImGui::TextUnformatted(file.FileTypeString.data());
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
						directoryToOpen = path;

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

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);
			if (ImGui::BeginPopupContextWindow("AssetPanelHierarchyContextWindow", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				EditorLayer::GetInstance()->ResetContext();
				DrawContextMenuItems(m_CurrentDirectory, true);
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			ImGui::EndTable();

			if (!anyItemHovered && ImGui::IsItemClicked())
				EditorLayer::GetInstance()->ResetContext();
		}

		ImGui::PopStyleVar();

		if (!directoryToDelete.empty())
		{
			std::filesystem::remove_all(directoryToDelete);
			EditorLayer::GetInstance()->ResetContext();
			Refresh();
		}

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
			std::string filename = relativePath.filename().string();
			std::string extension = relativePath.extension().string();

			auto fileType = FileType::Unknown;
			const auto& fileTypeIt = s_FileTypes.find(extension);
			if (fileTypeIt != s_FileTypes.end())
				fileType = fileTypeIt->second;

			std::string_view fileTypeString = s_FileTypesToString.at(FileType::Unknown);
			const auto& fileStringTypeIt = s_FileTypesToString.find(fileType);
			if (fileTypeIt != s_FileTypes.end())
				fileTypeString = fileStringTypeIt->second;

			ImVec4 fileTypeColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			const auto& fileTypeColorIt = s_TypeColors.find(fileType);
			if (fileTypeColorIt != s_TypeColors.end())
				fileTypeColor = fileTypeColorIt->second;

			m_DirectoryEntries.emplace_back(filename, path.string(), extension, directoryEntry, nullptr, directoryEntry.is_directory(),
				fileType, fileTypeString, fileTypeColor);
		}

		m_ElapsedTime = 0.0f;
	}

	void AssetPanel::DrawContextMenuItems(const std::filesystem::path& context, bool isDir)
	{
		if (isDir)
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Folder"))
				{
					int i = 0;
					bool created = false;
					std::string newFolderPath;
					while (!created)
					{
						std::string folderName = "New Folder" + (i == 0 ? "" : std::format(" ({})", i));
						newFolderPath = (context / folderName).string();
						created = std::filesystem::create_directory(newFolderPath);
						++i;
					}
					Refresh();
					EditorLayer::GetInstance()->SetContext(EditorContextType::File, newFolderPath.c_str(), sizeof(char) * (newFolderPath.length() + 1));
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("C# Script"))
				{
					Filesystem::WriteFileText(context / "Script.cs", "using ArcEngine;");
					Refresh();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndMenu();
			}
		}
		if (ImGui::MenuItem("Show in Explorer"))
		{
			FileDialogs::OpenFolderAndSelectItem(context.string().c_str());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Open"))
		{
			FileDialogs::OpenFileWithProgram(context.string().c_str());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Copy Path"))
		{
			ImGui::SetClipboardText(context.string().c_str());
			ImGui::CloseCurrentPopup();
		}

		if (isDir)
		{
			if (ImGui::MenuItem("Refresh"))
			{
				Refresh();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Open C# Project"))
			{
				VisualStudioAccessor::RunVisualStudio();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}
