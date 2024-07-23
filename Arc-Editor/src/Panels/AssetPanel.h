#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>

#include "BasePanel.h"

namespace ArcEngine
{
	enum class FileType
	{
		Unknown = 0,
		Scene, Prefab, Script, Shader,
		Texture, Cubemap, Model,
		Audio
	};

	class AssetPanel : public BasePanel
	{
	public:
		explicit AssetPanel(const char* name = "Assets");

		~AssetPanel() override = default;

		AssetPanel(const AssetPanel& other) = delete;
		AssetPanel(AssetPanel&& other) = delete;
		AssetPanel& operator=(const AssetPanel& other) = delete;
		AssetPanel& operator=(AssetPanel&& other) = delete;

		void OnUpdate(Timestep ts, WorkQueue* queue) override;
		void OnImGuiRender(WorkQueue* queue) override;

		void Invalidate();

	private:
		std::pair<bool, uint32_t> DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selectionMask, ImGuiTreeNodeFlags flags);
		void RenderHeader();
		void RenderSideView();
		void RenderBody(bool grid);
		void UpdateDirectoryEntries(const std::filesystem::path& directory);
		void Refresh() { UpdateDirectoryEntries(m_CurrentDirectory); }

		void DrawContextMenuItems(const std::filesystem::path& context, bool isDir);

	private:
		struct File
		{
			eastl::string Name;
			eastl::string Filepath;
			eastl::string Extension;
			std::filesystem::directory_entry DirectoryEntry;
			Ref<Texture2D> Thumbnail = nullptr;
			bool IsDirectory = false;

			FileType Type;
			std::string_view FileTypeString;
			ImVec4 FileTypeIndicatorColor;
		};

		std::filesystem::path m_AssetsDirectory;
		std::filesystem::path m_CurrentDirectory;
		eastl::stack<std::filesystem::path> m_BackStack;
		eastl::vector<File> m_DirectoryEntries;
		uint32_t m_CurrentlyVisibleItemsTreeView = 0;
		float m_ThumbnailSize = 128.0f;
		ImGuiTextFilter m_Filter;
		float m_ElapsedTime = 0.0f;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}
