#include "AssetPanel.h"

#include <Arc/Scene/EntitySerializer.h>
#include <Arc/Core/Filesystem.h>

#ifdef ARC_PLATFORM_VISUAL_STUDIO
#include <Platform/VisualStudio/VisualStudioAccessor.h>
#endif //ARC_PLATFORM_VISUAL_STUDIO

#include <EASTL/hash_map.h>
#include <Icons.h>
#include <imgui/imgui_internal.h>

#include "../EditorLayer.h"
#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

#if defined(__clang__) || defined(__llvm__)
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Weverything"
#elif defined(_MSC_VER)
#	pragma warning(push, 0)
#endif

#include "../Utils/FileWatch.hpp"

#if defined(__clang__) || defined(__llvm__)
#	pragma clang diagnostic pop
#elif defined(_MSC_VER)
#	pragma warning(pop)
#endif

namespace ArcEngine
{
	static const eastl::hash_map<FileType, const char*> s_FileTypesToString =
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

	static const eastl::hash_map<eastl::string, FileType> s_FileTypes =
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

	static const eastl::hash_map<FileType, ImVec4> s_TypeColors =
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

	static const eastl::hash_map<FileType, const char*> s_FileTypesToIcon =
	{
		{ FileType::Unknown,	ARC_ICON_FILE },

		{ FileType::Scene,		ARC_ICON_SCENE },
		{ FileType::Prefab,		ARC_ICON_PREFAB },
		{ FileType::Script,		ARC_ICON_LANGUAGE_CSHARP },
		{ FileType::Shader,		ARC_ICON_SHADER },

		{ FileType::Texture,	ARC_ICON_TEXTURE },
		{ FileType::Cubemap,	ARC_ICON_CUBEMAP },
		{ FileType::Model,		ARC_ICON_POLYGON },

		{ FileType::Audio,		ARC_ICON_AUDIO },
	};

	inline static float s_LastDomainReloadTime = 0.0f;
	
	static void DragDropTarget(const eastl::string_view filepath)
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
		{
			const Entity entity = *static_cast<Entity*>(payload->Data);
			eastl::string path(filepath);
			path += "/" + entity.GetComponent<TagComponent>().Tag + ".prefab";
			EntitySerializer::SerializeEntityAsPrefab(path.c_str(), entity);
		}
	}

	static void DragDropFrom(const eastl::string_view filepath, const eastl::string_view tooltip)
	{
		ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", filepath.begin(), filepath.size() + 1);
		ImGui::TextUnformatted(tooltip.begin(), tooltip.end());
		ImGui::EndDragDropSource();
	}

	static void OpenFile(const std::filesystem::path& path)
	{
		const std::string filepathString = path.string();
		const char* filepath = filepathString.c_str();
		const std::string ext = path.extension().string();
		const auto& fileTypeIt = s_FileTypes.find_as(ext.c_str());
		if (fileTypeIt != s_FileTypes.end())
		{
			const FileType fileType = fileTypeIt->second;
			switch (fileType)
			{
				case FileType::Scene:
					EditorLayer::GetInstance()->OpenScene(filepath);
					break;
				case FileType::Script: [[fallthrough]];
				case FileType::Shader:
#ifdef ARC_PLATFORM_VISUAL_STUDIO
					VisualStudioAccessor::OpenFile(filepath);
#endif //ARC_PLATFORM_VISUAL_STUDIO
					break;
				case FileType::Unknown:
				case FileType::Prefab:
				case FileType::Texture:
				case FileType::Cubemap:
				case FileType::Model: [[fallthrough]];
				case FileType::Audio:
					FileDialogs::OpenFileWithProgram(filepath);
					break;
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

			const bool entryIsFile = !std::filesystem::is_directory(entryPath);
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

			const uint64_t id = *count;
			bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(id), nodeFlags, "");
			ImGui::PopStyleColor(selected ? 2 : 1);

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				if (!entryIsFile)
					UpdateDirectoryEntries(entryPath);

				nodeClicked = *count;
				anyNodeClicked = true;
			}

			const std::string filepath = entryPath.string();
			eastl::string_view name = StringUtils::GetNameWithExtension(filepath.c_str());
			if (ImGui::BeginDragDropTarget())
			{
				if (!entryIsFile)
					DragDropTarget(filepath.c_str());

				ImGui::EndDragDropTarget();
			}
			if (ImGui::BeginDragDropSource())
			{
				DragDropFrom(filepath.c_str(), name);
			}

			const char* folderIcon = ARC_ICON_FILE;
			if (entryIsFile)
			{
				auto fileType = FileType::Unknown;
				auto entryPathStr = entryPath.extension().string();
				const auto& fileTypeIt = s_FileTypes.find_as(entryPathStr.c_str());
				if (fileTypeIt != s_FileTypes.end())
					fileType = fileTypeIt->second;

				const auto& fileTypeIconIt = s_FileTypesToIcon.find(fileType);
				if (fileTypeIconIt != s_FileTypesToIcon.end())
					folderIcon = fileTypeIconIt->second;
			}
			else
			{
				folderIcon = open ? ARC_ICON_FOLDER_OPEN : ARC_ICON_FOLDER;
			}

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
			ImGui::TextUnformatted(folderIcon);
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::TextUnformatted(name.begin(), name.end());
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
					for ([[maybe_unused]] const auto& e : std::filesystem::recursive_directory_iterator(entryPath))
						(*count)--;
				}
			}
		}

		return { anyNodeClicked, nodeClicked };
	}

	AssetPanel::AssetPanel(const char* name)
		: BasePanel(name, ARC_ICON_FOLDER_STAR, true)
	{
		ARC_PROFILE_SCOPE();

		m_DirectoryIcon = AssetManager::GetTexture2D("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = AssetManager::GetTexture2D("Resources/Icons/ContentBrowser/FileIcon.png");

		m_AssetsDirectory = Project::GetAssetDirectory();
		m_CurrentDirectory = m_AssetsDirectory;
		Refresh();

		static filewatch::FileWatch<std::string> watch(m_AssetsDirectory.string(),
			[this](const auto& path, const filewatch::Event change_type)
			{
				if (s_LastDomainReloadTime < 0.1f)
					return;

				Application::Get().SubmitToMainThread([this]() { Refresh(); });

				std::filesystem::path filepath = path;
				const std::string ext = filepath.extension().string();
				FileType fileType = FileType::Unknown;
				if (s_FileTypes.find_as(ext.c_str()) != s_FileTypes.end())
					fileType = s_FileTypes.at(ext.c_str());

				if (fileType == FileType::Script)
				{
					s_LastDomainReloadTime = 0.0f;
					Application::Get().SubmitToMainThread([]() { ScriptEngine::ReloadAppDomain(); });
				}

				switch (change_type)
				{
					case filewatch::Event::added:
						ARC_CORE_TRACE("The file was added to the directory: {}", filepath);
						break;
					case filewatch::Event::removed:
						ARC_CORE_TRACE("The file was removed from the directory: {}", filepath);
						break;
					case filewatch::Event::modified:
						ARC_CORE_TRACE("The file was modified: {}; This can be a change in the time stamp or attributes.", filepath);
						break;
					case filewatch::Event::renamed_old:
						ARC_CORE_TRACE("The file was renamed and this is the old name: {}", filepath);
						break;
					case filewatch::Event::renamed_new:
						ARC_CORE_TRACE("The file was renamed and this is the new name: {}", filepath);
						break;
				}
			}
		);
	}

	void AssetPanel::OnUpdate(Timestep ts, WorkQueue* queue)
	{
		ARC_PROFILE_SCOPE();

		m_ElapsedTime += ts;
		s_LastDomainReloadTime += ts;

		if (ImGui::IsKeyPressed(ImGuiKey_Delete) && GImGui->ActiveId == 0)
		{
			const EditorContext& context = EditorLayer::GetInstance()->GetContext();
			if (context.IsValid(EditorContextType::File))
			{
				const std::filesystem::path path = context.As<char>();
				std::filesystem::remove_all(path);
				EditorLayer::GetInstance()->ResetContext();
			}
		}
	}

	void AssetPanel::OnImGuiRender(WorkQueue* queue)
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
			const ImVec2 availableRegion = ImGui::GetContentRegionAvail();
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

		if (ImGui::Button(ARC_ICON_COGS))
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
			static const char* searchString = ARC_ICON_SEARCH " Search...";
			static const char* searchStringEnd = searchString + strlen(searchString);
			ImGui::TextUnformatted(searchString, searchStringEnd);
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

			if (ImGui::Button(ARC_ICON_ARROW_LEFT_CIRCLE))
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

			if (ImGui::Button(ARC_ICON_ARROW_RIGHT_CIRCLE))
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

		constexpr const char* folderIcon = ARC_ICON_FOLDER;
		ImGui::TextUnformatted(folderIcon);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 0.0f });

		std::filesystem::path projectDir = m_AssetsDirectory.parent_path();
		static std::filesystem::path absProjectDir = std::filesystem::absolute(m_AssetsDirectory.parent_path());
		static std::string absProjectDirString = absProjectDir.string();
		static size_t absProjectDirLength = absProjectDirString.length();

		std::string currentDir = m_CurrentDirectory.string();
		const char* p = &currentDir[absProjectDirLength + 1];
		const std::filesystem::path currentDirectory = p;

		std::filesystem::path directoryToOpen;
		for (auto& path : currentDirectory)
		{
			projectDir /= path;
			ImGui::SameLine();
			if (ImGui::Button(path.filename().string().c_str()))
				directoryToOpen = projectDir;

			if (m_CurrentDirectory != projectDir)
			{
				ImGui::SameLine();
				constexpr const char* delimeter = "/";
				ImGui::TextUnformatted(delimeter, delimeter + 1);
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

			const bool opened = ImGui::TreeNodeEx("AssetsDir", nodeFlags, "");
			ImGui::PopStyleColor(selected ? 2 : 1);

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				UpdateDirectoryEntries(m_AssetsDirectory);
				selectionMask = 0;
			}
			const char* folderIcon = opened ? ARC_ICON_FOLDER_OPEN : ARC_ICON_FOLDER;
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
			ImGui::TextUnformatted(folderIcon);
			ImGui::PopStyleColor();
			ImGui::SameLine();
			constexpr char rootDirName[] = "Assets";
			ImGui::TextUnformatted(rootDirName, rootDirName + sizeof(rootDirName) - 1);

			if (opened)
			{
				uint32_t count = 0;
				for ([[maybe_unused]] const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetsDirectory))
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

		std::filesystem::path directoryToOpen;
		std::filesystem::path directoryToDelete;

		constexpr float padding = 4.0f;
		const float scaledThumbnailSize = m_ThumbnailSize * ImGui::GetIO().FontGlobalScale;
		const float scaledThumbnailSizeX = scaledThumbnailSize * 0.55f;
		const float cellSize = scaledThumbnailSizeX + 2 * padding + scaledThumbnailSizeX * 0.1f;

		constexpr float overlayPaddingY = 6.0f * padding;
		constexpr float thumbnailPadding = overlayPaddingY * 0.5f;
		const float thumbnailSize = scaledThumbnailSizeX - thumbnailPadding;

		const ImVec2 backgroundThumbnailSize = { scaledThumbnailSizeX + padding * 2, scaledThumbnailSize + padding * 2 };

		const float panelWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ScrollbarSize;
		int columnCount = static_cast<int>(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		float lineHeight = ImGui::GetTextLineHeight();
		int flags = ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_ScrollY;

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
		const ImVec2 region = ImGui::GetContentRegionAvail();
		ImGui::InvisibleButton("##DragDropTargetAssetPanelBody", region);
		
		ImGui::SetNextItemAllowOverlap();
		ImGui::SetCursorPos(cursorPos);

		if (ImGui::BeginTable("BodyTable", columnCount, flags))
		{
			bool anyItemHovered = false;
			bool textureCreated = false;

			int i = 0;
			for (auto& file : m_DirectoryEntries)
			{
				ImGui::PushID(i);

				const bool isDir = file.IsDirectory;
				const char* filename = file.Name.data();
				const char* filenameEnd = filename + file.Name.size();

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
							file.Thumbnail = AssetManager::GetTexture2D(file.Filepath.c_str());
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

				if (textureId == 0)
				{
					textureId = AssetManager::BlackTexture()->GetRendererID();
				}

				ImGui::TableNextColumn();

				const auto& path = file.DirectoryEntry.path();

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
					static eastl::string id = "###";
					id[2] = static_cast<char>(i);
					bool const clicked = UI::ToggleButton(id, highlight, backgroundThumbnailSize, 0.0f, 1.0f);
					if (m_ElapsedTime > 0.25f && clicked)
					{
						EditorLayer::GetInstance()->SetContext(EditorContextType::File, file.Filepath.c_str(), sizeof(char) * (file.Filepath.length() + 1));
					}
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);
					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::MenuItemEx("Delete", ARC_ICON_DELETE))
						{
							directoryToDelete = path;
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::MenuItemEx("Rename", ARC_ICON_RENAME))
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::Separator();

						DrawContextMenuItems(path, isDir);
						ImGui::EndPopup();
					}
					ImGui::PopStyleVar();

					if (ImGui::BeginDragDropTarget())
					{
						if (isDir)
							DragDropTarget(file.Filepath);

						ImGui::EndDragDropTarget();
					}
					if (ImGui::BeginDragDropSource())
					{
						DragDropFrom(file.Filepath, file.Name);
					}

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

					const ImTextureID whiteTexId = reinterpret_cast<ImTextureID>(AssetManager::WhiteTexture()->GetRendererID());

					// Foreground Image
					ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
					ImGui::SetNextItemAllowOverlap();
					ImGui::Image(whiteTexId, { backgroundThumbnailSize.x - padding * 2.0f, backgroundThumbnailSize.y - padding * 2.0f }, { 0, 0 }, { 1, 1 }, EditorTheme::WindowBgAlternativeColor);

					// Thumbnail Image
					ImGui::SetCursorPos({ cursorPos.x + thumbnailPadding * 0.75f, cursorPos.y + thumbnailPadding });
					ImGui::SetNextItemAllowOverlap();
					ImGui::Image(reinterpret_cast<ImTextureID>(textureId), { thumbnailSize, thumbnailSize }, ARC_UI_UV_0, ARC_UI_UV_1);

					// Type Color frame
					const ImVec2 typeColorFrameSize = { scaledThumbnailSizeX, scaledThumbnailSizeX * 0.03f };
					ImGui::SetCursorPosX(cursorPos.x + padding);
					ImGui::Image(whiteTexId, typeColorFrameSize, { 0, 0 }, { 1, 1 }, isDir ? ImVec4(0.0f, 0.0f, 0.0f, 0.0f) : file.FileTypeIndicatorColor);

					const ImVec2 rectMin = ImGui::GetItemRectMin();
					const ImVec2 rectSize = ImGui::GetItemRectSize();
					const ImRect clipRect = ImRect({ rectMin.x + padding * 1.0f, rectMin.y + padding * 2.0f },
						{ rectMin.x + rectSize.x, rectMin.y + scaledThumbnailSizeX - EditorTheme::SmallFont->FontSize - padding * 4.0f });
					UI::ClippedText(clipRect.Min, clipRect.Max, filename, filenameEnd, nullptr, { 0, 0 }, nullptr, clipRect.GetSize().x);

					if (!isDir)
					{
						ImGui::SetCursorPos({ cursorPos.x + padding * 2.0f, cursorPos.y + backgroundThumbnailSize.y - EditorTheme::SmallFont->FontSize - padding * 2.0f });
						ImGui::BeginDisabled();
						ImGui::PushFont(EditorTheme::SmallFont);
						ImGui::TextUnformatted(file.FileTypeString._Unchecked_begin(), file.FileTypeString._Unchecked_end());
						ImGui::PopFont();
						ImGui::EndDisabled();
					}
				}
				else
				{
					constexpr ImGuiTreeNodeFlags teeNodeFlags = ImGuiTreeNodeFlags_FramePadding
						| ImGuiTreeNodeFlags_SpanFullWidth
						| ImGuiTreeNodeFlags_Leaf;

					const bool opened = ImGui::TreeNodeEx(file.Name.c_str(), teeNodeFlags, "");

					if (ImGui::IsItemHovered())
						anyItemHovered = true;

					if (isDir && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						directoryToOpen = path;

					if (ImGui::BeginDragDropSource())
					{
						DragDropFrom(file.Filepath, file.Name);
					}

					ImGui::SameLine();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - lineHeight);
					ImGui::Image(reinterpret_cast<ImTextureID>(textureId), { lineHeight, lineHeight }, ARC_UI_UV_0, ARC_UI_UV_1);
					ImGui::SameLine();
					ImGui::TextUnformatted(filename, filenameEnd);

					if (opened)
						ImGui::TreePop();
				}

				ImGui::PopID();
				++i;
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
		}

		if (!directoryToOpen.empty())
			UpdateDirectoryEntries(directoryToOpen);
	}

	void AssetPanel::UpdateDirectoryEntries(const std::filesystem::path& directory)
	{
		ARC_PROFILE_SCOPE();

		m_CurrentDirectory = directory;
		m_DirectoryEntries.clear();
		const auto directoryIt = std::filesystem::directory_iterator(directory);
		for (auto& directoryEntry : directoryIt)
		{
			const auto& path = directoryEntry.path();
			const auto relativePath = std::filesystem::relative(path, m_AssetsDirectory);
			const std::string filename = relativePath.filename().string();
			const std::string filepath = path.string();
			const std::string extension = relativePath.extension().string();

			auto fileType = FileType::Unknown;
			const auto& fileTypeIt = s_FileTypes.find_as(extension.c_str());
			if (fileTypeIt != s_FileTypes.end())
				fileType = fileTypeIt->second;

			std::string_view fileTypeString = s_FileTypesToString.at(FileType::Unknown);
			const auto& fileStringTypeIt = s_FileTypesToString.find(fileType);
			if (fileStringTypeIt != s_FileTypesToString.end())
				fileTypeString = fileStringTypeIt->second;

			ImVec4 fileTypeColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			const auto& fileTypeColorIt = s_TypeColors.find(fileType);
			if (fileTypeColorIt != s_TypeColors.end())
				fileTypeColor = fileTypeColorIt->second;

			m_DirectoryEntries.push_back({ filename.c_str(), filepath.c_str(), extension.c_str(), directoryEntry, nullptr, directoryEntry.is_directory(),
				fileType, fileTypeString, fileTypeColor });
		}

		m_ElapsedTime = 0.0f;
	}

	void AssetPanel::DrawContextMenuItems(const std::filesystem::path& context, bool isDir)
	{
		if (isDir)
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItemEx("Folder", ARC_ICON_FOLDER_PLUS))
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
					EditorLayer::GetInstance()->SetContext(EditorContextType::File, newFolderPath.c_str(), sizeof(char) * (newFolderPath.length() + 1));
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItemEx("Script", ARC_ICON_LANGUAGE_CSHARP))
				{
					Filesystem::WriteFileText(context / "Script.cs", "using ArcEngine;");
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndMenu();
			}
		}
		if (ImGui::MenuItemEx("Show in Explorer", ARC_ICON_ARROW_TOP_RIGHT))
		{
			std::string path = context.string();
			FileDialogs::OpenFolderAndSelectItem(path.c_str());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItemEx("Open", ARC_ICON_OPEN_IN_APP))
		{
			std::string path = context.string();
			FileDialogs::OpenFileWithProgram(path.c_str());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Copy Path"))
		{
			std::string path = context.string();
			ImGui::SetClipboardText(path.c_str());
			ImGui::CloseCurrentPopup();
		}

		if (isDir)
		{
			if (ImGui::MenuItemEx("Refresh", ARC_ICON_REFRESH))
			{
				Refresh();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Open C# Project"))
			{
#ifdef ARC_PLATFORM_VISUAL_STUDIO
				VisualStudioAccessor::RunVisualStudio();
#endif //ARC_PLATFORM_VISUAL_STUDIO
				ImGui::CloseCurrentPopup();
			}
		}
	}
}
