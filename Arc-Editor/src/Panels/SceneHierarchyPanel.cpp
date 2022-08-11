#include "SceneHierarchyPanel.h"

#include <Arc/Scene/EntitySerializer.h>

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

	static void DragDropTarget(Scene& scene)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				eastl::string ext = StringUtils::GetExtension(path);
				if (ext == "prefab")
					EntitySerializer::DeserializeEntityAsPrefab(path, scene);
			}

			ImGui::EndDragDropTarget();
		}
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

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg
			| ImGuiTableFlags_ContextMenuInBody
			| ImGuiTableFlags_BordersInner
			| ImGuiTableFlags_ScrollY;

		ImVec2 cellPadding = ImGui::GetStyle().CellPadding;
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });

		if (OnBegin(ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
		{
			float lineHeight = ImGui::GetTextLineHeight();
			ImVec2 padding = ImGui::GetStyle().FramePadding;

			const char* addButtonLabel = "  " ICON_MDI_PLUS "Add  ";
			ImVec2 addButtonSize = ImGui::CalcTextSize(addButtonLabel);
			addButtonSize.x += padding.y * 2.0f;
			addButtonSize.y = lineHeight + padding.y * 2.0f;

			const float filterCursorPosX = ImGui::GetCursorPosX();
			m_Filter.Draw("###HierarchyFilter", ImGui::GetContentRegionAvail().x - (addButtonSize.x + padding.x * 2.0f));
			
			ImGui::SameLine();

			if (UI::IconButton("  " ICON_MDI_PLUS, "Add  ", { 0.537f, 0.753f, 0.286f, 1.0f }))
				ImGui::OpenPopup("SceneHierarchyContextWindow");

			if (!m_Filter.IsActive())
			{
				ImGui::SameLine();
				ImGui::SetCursorPosX(filterCursorPosX + ImGui::GetFontSize() * 0.5f);
				ImGui::TextUnformatted(ICON_MDI_MAGNIFY " Search...");
			}

			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 region = ImGui::GetContentRegionAvail();
			ImGui::InvisibleButton("##DragDropTargetBehindTable", region);
			DragDropTarget(*m_Context);

			ImGui::SetCursorPos(cursorPos);

			if (ImGui::BeginTable("HierarchyTable", 3, tableFlags))
			{
				ImGui::TableSetupColumn("  Label", ImGuiTableColumnFlags_NoHide);
				ImGui::TableSetupColumn("  Type", ImGuiTableColumnFlags_WidthFixed, lineHeight * 3.0f);
				ImGui::TableSetupColumn("  " ICON_MDI_EYE_OUTLINE, ImGuiTableColumnFlags_WidthFixed, lineHeight * 2.0f);

				ImGui::TableSetupScrollFreeze(0, 1);
				
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers, ImGui::GetFrameHeight());
				for (int column = 0; column < 3; ++column)
				{
					ImGui::TableSetColumnIndex(column);
					const char* column_name = ImGui::TableGetColumnName(column);
					ImGui::PushID(column);
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.y);
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

				if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
					m_SelectionContext = {};

				if (ImGui::BeginPopupContextWindow("SceneHierarchyContextWindow", 1, false))
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

				ImGui::EndTable();
			}

			if (m_DraggedEntity && m_DraggedEntityTarget)
			{
				m_DraggedEntity.SetParent(m_DraggedEntityTarget);
				m_DraggedEntity = {};
				m_DraggedEntityTarget = {};
			}

			if (m_DeletedEntity)
			{
				if (m_SelectionContext == m_DeletedEntity)
					m_SelectionContext = {};

				m_Context->DestroyEntity(m_DeletedEntity);
				m_DeletedEntity = {};
			}

			OnEnd();
		}
		ImGui::PopStyleVar();
	}

	ImRect SceneHierarchyPanel::DrawEntityNode(Entity entity, bool skipChildren, uint32_t depth, bool forceExpandTree, bool isPartOfPrefab)
	{
		ARC_PROFILE_SCOPE();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		auto& tagComponent = entity.GetComponent<TagComponent>();
		auto& tag = tagComponent.Tag;

		auto& rc = entity.GetRelationship();
		uint32_t childrenSize = rc.Children.size();

		if (m_Filter.IsActive() && !m_Filter.PassFilter(tag.c_str()))
		{
			for (size_t i = 0; i < childrenSize; i++)
			{
				UUID childId = entity.GetRelationship().Children[i];
				Entity child = m_Context->GetEntity(childId);
				DrawEntityNode(child);
			}
			return { 0, 0, 0, 0 };
		}

		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0);
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth;
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

		if (forceExpandTree)
			ImGui::SetNextItemOpen(true);
		
		if (!isPartOfPrefab)
			isPartOfPrefab = entity.HasComponent<PrefabComponent>();
		bool prefabColorApplied = isPartOfPrefab && entity != m_SelectionContext;
		if (prefabColorApplied)
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::HeaderSelectedColor);

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, "%s %s", ICON_MDI_CUBE_OUTLINE, tag.c_str());

		if (ImGui::IsItemToggledOpen() && (ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt)))
			forceExpandTree = opened;

		if (ImGui::IsItemHovered())
		{
			if ((ImGui::IsMouseDown(0) || ImGui::IsMouseDown(1)) && !ImGui::IsItemToggledOpen())
				m_SelectionContext = entity;
		}

		if (highlight)
			ImGui::PopStyleColor(2);

		bool entityDeleted = false;
		bool createChild = false;
		static Entity renaming = {};
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Create"))
				createChild = true;
			if (ImGui::MenuItem("Rename"))
				renaming = entity;
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}
		ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
		verticalLineStart.y -= ImGui::GetFrameHeight() * 0.5f;

		// Drag Drop
		{
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					Entity* e = (Entity*)payload->Data;
					m_DraggedEntity = *e;
					m_DraggedEntityTarget = entity;
				}

				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("Entity", (void*)&entity, sizeof(entity));
				ImGui::Text(tag.c_str());
				ImGui::EndDragDropSource();
			}
		}

		if (entity == renaming)
		{
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				tag = eastl::string(buffer);

			if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
				renaming = {};
		}

		ImGui::TableNextColumn();
		ImGui::Text("  %s", isPartOfPrefab ? "Prefab" : "Entity");

		ImGui::TableNextColumn();
		ImGui::Text("  %s", tagComponent.Enabled ? ICON_MDI_EYE_OUTLINE : ICON_MDI_EYE_OFF_OUTLINE);

		if (prefabColorApplied)
			ImGui::PopStyleColor();

		if (!ImGui::IsItemHovered())
			tagComponent.handled = false;

		if ((!tagComponent.handled && ImGui::IsItemHovered() && ImGui::IsMouseDragging(0)) || ImGui::IsItemClicked(0))
		{
			tagComponent.handled = true;
			tagComponent.Enabled = !tagComponent.Enabled;
		}

		// Open
		const ImRect nodeRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		{
			if (opened && !skipChildren && !entityDeleted)
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
					const ImRect childRect = DrawEntityNode(child, createChild, depth + 1, forceExpandTree, isPartOfPrefab);

					const float midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;
					drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + HorizontalTreeLineSize, midpoint), treeLineColor);
					verticalLineEnd.y = midpoint;
				}

				drawList->AddLine(verticalLineStart, verticalLineEnd, treeLineColor);
			}

			if (opened && childrenSize > 0)
				ImGui::TreePop();
		}

		// PostProcess Actions
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
