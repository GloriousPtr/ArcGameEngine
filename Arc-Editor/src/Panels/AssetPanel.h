#pragma once

#include <EASTL/stack.h>
#include <imgui/imgui.h>

#include "BasePanel.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	class AssetPanel : public BasePanel
	{
	public:
		AssetPanel(const char* name = "Assets");

		virtual ~AssetPanel() override = default;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;

	private:
		std::pair<bool, uint32_t> DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selection_mask, ImGuiTreeNodeFlags flags);
		void RenderHeader();
		void RenderSideView();
		void RenderBody(bool grid);
		void UpdateDirectoryEntries(std::filesystem::path directory);

	private:
		struct File
		{
			eastl::string Name;
			std::filesystem::directory_entry DirectoryEntry;
		};

		std::filesystem::path m_CurrentDirectory;
		eastl::stack<std::filesystem::path> m_BackStack;
		eastl::vector<File> m_DirectoryEntries;
		uint32_t m_CurrentlyVisibleItemsTreeView = 0;
		float m_ThumbnailSize = 64.0f;
		ImGuiTextFilter m_Filter;
		float m_ElapsedTime = 0.0f;

		Ref<Texture2D> m_WhiteTexture;
		Ref<Texture2D> m_DirectoryIcon;
	};
}
