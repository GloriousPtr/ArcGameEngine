#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		ARC_PROFILE_SCOPE();

		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg
			| ImGuiTableFlags_ContextMenuInBody
			| ImGuiTableFlags_BordersInner;

		ImVec2 cellPadding = ImGui::GetStyle().CellPadding;
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });

		if (OnBegin())
		{
			const float cursorPosX = ImGui::GetCursorPosX();
			m_Filter.Draw("###ConsoleFilter", ImGui::GetContentRegionAvail().x);
			if (!m_Filter.IsActive())
			{
				ImGui::SameLine();
				ImGui::SetCursorPosX(cursorPosX + ImGui::GetFontSize() * 0.5f);
				ImGui::TextUnformatted(ICON_MDI_MAGNIFY " Search...");
			}

			if (ImGui::BeginTable("HierarchyTable", 3, tableFlags))
			{
				float lineHeight = ImGui::GetTextLineHeight();
				ImGui::TableSetupColumn("  Label", ImGuiTableColumnFlags_NoHide);
				ImGui::TableSetupColumn("  Type", ImGuiTableColumnFlags_WidthFixed, lineHeight * 3.0f);
				ImGui::TableSetupColumn("  " ICON_MDI_EYE_OUTLINE, ImGuiTableColumnFlags_WidthFixed, lineHeight * 2.0f);
				
				float paddingY = ImGui::GetStyle().FramePadding.y;
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers, ImGui::GetFrameHeight());
				for (int column = 0; column < 3; ++column)
				{
					ImGui::TableSetColumnIndex(column);
					const char* column_name = ImGui::TableGetColumnName(column);
					ImGui::PushID(column);
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + paddingY);
					ImGui::TableHeader(column_name);
					ImGui::PopID();
				}

				auto view = m_Context->m_Registry.view<IDComponent>();
				for (auto it = view.rbegin(); it != view.rend(); it++)
				{
					entt::entity e = *it;
					if (!view.contains(e))
						break;

					Entity entity = { e, m_Context.get() };
					if (!entity.GetParent())
						DrawEntityNode(entity);
				}

				ImGui::EndTable();
			}

			if (m_DeletedEntity)
			{
				if (m_SelectionContext == m_DeletedEntity)
					m_SelectionContext = {};

				m_Context->DestroyEntity(m_DeletedEntity);
				m_DeletedEntity = {};
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};

			if (ImGui::BeginPopupContextWindow(0, 1, false))
			{
				m_SelectionContext = {};
				if (ImGui::BeginMenu("Create"))
				{
					if (ImGui::MenuItem("Empty Entity"))
					{
						m_SelectionContext = m_Context->CreateEntity("Empty Entity");
					}
					else if (ImGui::MenuItem("Camera"))
					{
						m_SelectionContext = m_Context->CreateEntity("Camera");
						m_SelectionContext.AddComponent<CameraComponent>();
						ImGui::CloseCurrentPopup();
					}
					else if (ImGui::MenuItem("Sprite"))
					{
						m_SelectionContext = m_Context->CreateEntity("Sprite");
						m_SelectionContext.AddComponent<SpriteRendererComponent>();
						ImGui::CloseCurrentPopup();
					}
					else if (ImGui::MenuItem("Mesh"))
					{
						m_SelectionContext = m_Context->CreateEntity("Mesh");
						m_SelectionContext.AddComponent<MeshComponent>();
						ImGui::CloseCurrentPopup();
					}
					else if (ImGui::MenuItem("Sky Light"))
					{
						m_SelectionContext = m_Context->CreateEntity("Sky Light");
						m_SelectionContext.AddComponent<SkyLightComponent>();
						ImGui::CloseCurrentPopup();
					}
					else if (ImGui::MenuItem("Light"))
					{
						m_SelectionContext = m_Context->CreateEntity("Light");
						m_SelectionContext.AddComponent<LightComponent>();
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				ImGui::EndPopup();
			}
			OnEnd();
		}
		ImGui::PopStyleVar();
	}

	ImRect SceneHierarchyPanel::DrawEntityNode(Entity entity, bool skipChildren, uint32_t depth)
	{
		ARC_PROFILE_SCOPE();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		auto& tagComponent = entity.GetComponent<TagComponent>();
		auto& tag = tagComponent.Tag;
		auto& rc = entity.GetRelationship();
		uint32_t childrenSize = rc.Children.size();

		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0);
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		flags |= ImGuiTreeNodeFlags_FramePadding;

		if (childrenSize == 0)
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		bool highlight = m_SelectionContext == entity;
		if (highlight)
		{
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(EditorTheme::HeaderSelectedColor));
			ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::HeaderSelectedColor);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderSelectedColor);
		}

		eastl::string displayName = (ICON_MDI_CUBE_OUTLINE + eastl::string(" ") + tag);
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, displayName.c_str());
		if (highlight)
			ImGui::PopStyleColor(2);

		if (ImGui::IsItemHovered())
		{
			if ((ImGui::IsMouseDown(0) || ImGui::IsMouseDown(1)) && !ImGui::IsItemToggledOpen())
				m_SelectionContext = entity;
		}

		bool entityDeleted = false;
		bool createChild = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Create"))
				createChild = true;
			if (ImGui::MenuItem("Rename"))
				tagComponent.renaming = true;
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}
		ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
		verticalLineStart.y -= ImGui::GetFrameHeight() * 0.5f;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted("  Entity");

		ImGui::TableNextColumn();
		ImGui::TextUnformatted("  " ICON_MDI_EYE_OUTLINE);

		if (tagComponent.renaming)
		{
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				tag = eastl::string(buffer);

			if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
				tagComponent.renaming = false;
		}

		const ImRect nodeRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		if(opened && !skipChildren && !entityDeleted)
		{
			ImColor treeLineColor = ImColor(128, 128, 128, 128);

			depth %= 4;
			switch (depth)
			{
			case 0:
				treeLineColor = ImColor(254, 112, 246);
				break;
			case 1:
				treeLineColor = ImColor(142, 112, 254);
				break;
			case 2:
				treeLineColor = ImColor(112, 180, 254);
				break;
			case 3:
				treeLineColor = ImColor(48, 134, 198);
				break;
			}

			ImDrawList* drawList = ImGui::GetWindowDrawList();

			ImVec2 verticalLineEnd = verticalLineStart;

			for (size_t i = 0; i < childrenSize; i++)
			{
				UUID childId = entity.GetRelationship().Children[i];
				Entity child = m_Context->GetEntity(childId);
				const float HorizontalTreeLineSize = child.GetRelationship().Children.size() == 0 ? 18.0f : 9.0f; //chosen arbitrarily
	            const ImRect childRect = DrawEntityNode(child, createChild, depth + 1);
				const float midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;
			    drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + HorizontalTreeLineSize, midpoint), treeLineColor);
				verticalLineEnd.y = midpoint;
			}

			drawList->AddLine(verticalLineStart, verticalLineEnd, treeLineColor);
		}

		if (opened && childrenSize > 0)
			ImGui::TreePop();

		if (createChild)
		{
			Entity e = m_Context->CreateEntity();
			e.SetParent(entity);
		}

		if(entityDeleted)
		{
			m_DeletedEntity = entity;
		}

		return nodeRect;
	}
}
