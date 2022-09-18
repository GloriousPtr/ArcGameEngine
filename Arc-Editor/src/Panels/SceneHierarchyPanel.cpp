#include "SceneHierarchyPanel.h"

#include <Arc/Scene/EntitySerializer.h>
#include <Arc/Scene/EntitySerializer.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "../EditorLayer.h"
#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		ARC_PROFILE_SCOPE();

		m_Context = context;
	}

	void SceneHierarchyPanel::DragDropTarget() const
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				eastl::string name = StringUtils::GetName(path);
				eastl::string ext = StringUtils::GetExtension(path);

				if (ext == "prefab")
				{
					EntitySerializer::DeserializeEntityAsPrefab(path, *m_Context);
				}
				else if (ext == "arc")
				{
					EditorLayer::GetInstance()->OpenScene(path);
				}
				else if (ext == "hdr")
				{
					m_Context->CreateEntity(name).AddComponent<SkyLightComponent>()
						.Texture = AssetManager::GetTextureCubemap(path);
				}
				else if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
				{
					m_Context->CreateEntity(name).AddComponent<SpriteRendererComponent>()
						.Texture = AssetManager::GetTexture2D(path);
				}
				else if (ext == "assbin" || ext == "obj" || ext == "fbx")
				{
					Ref<Mesh> mesh = AssetManager::GetMesh(path);

					Entity parent = m_Context->CreateEntity(mesh->GetName());

					size_t meshCount = mesh->GetSubmeshCount();
					for (size_t i = 0; i < meshCount; i++)
					{
						const auto& submesh = mesh->GetSubmesh(i);
						Entity entity = m_Context->CreateEntity(submesh.Name);
						entity.SetParent(parent);
						auto& meshComponent = entity.AddComponent<MeshComponent>();
						meshComponent.Filepath = path;
						meshComponent.MeshGeometry = mesh;
						meshComponent.SubmeshIndex = i;
					}
				}
			}

			ImGui::EndDragDropTarget();
		}
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg
			| ImGuiTableFlags_ContextMenuInBody
			| ImGuiTableFlags_BordersInner
			| ImGuiTableFlags_ScrollY;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });

		if (OnBegin(ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
		{
			float lineHeight = ImGui::GetTextLineHeight();
			ImVec2 padding = ImGui::GetStyle().FramePadding;

			const float filterCursorPosX = ImGui::GetCursorPosX();
			m_Filter.Draw("###HierarchyFilter", ImGui::GetContentRegionAvail().x - (UI::GetIconButtonSize("  " ICON_MDI_PLUS, "Add  ").x + 2.0f * padding.x));
			
			ImGui::SameLine();

			if (UI::IconButton("  " ICON_MDI_PLUS, "Add  "))
				ImGui::OpenPopup("SceneHierarchyContextWindow");

			DrawContextMenu();

			if (!m_Filter.IsActive())
			{
				ImGui::SameLine();
				ImGui::SetCursorPosX(filterCursorPosX + ImGui::GetFontSize() * 0.5f);
				ImGui::TextUnformatted(ICON_MDI_MAGNIFY " Search...");
			}

			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 region = ImGui::GetContentRegionAvail();
			ImGui::InvisibleButton("##DragDropTargetBehindTable", region);
			DragDropTarget();

			ImGui::SetCursorPos(cursorPos);

			if (ImGui::BeginTable("HierarchyTable", 3, tableFlags))
			{
				ImGui::TableSetupColumn("  Label", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip);
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

				DrawContextMenu();

				ImGui::EndTable();

				if (ImGui::IsItemClicked())
					EditorLayer::GetInstance()->ResetContext();
			}

			if (m_DraggedEntity && m_DraggedEntityTarget)
			{
				m_DraggedEntity.SetParent(m_DraggedEntityTarget);
				m_DraggedEntity = {};
				m_DraggedEntityTarget = {};
			}

			if (m_DeletedEntity)
			{
				const EditorContext& context = EditorLayer::GetInstance()->GetContext();
				if (context.IsValid(EditorContextType::Entity) && *((Entity*)context.Data) == m_DeletedEntity)
					EditorLayer::GetInstance()->ResetContext();

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

		const auto& rc = entity.GetRelationship();
		size_t childrenSize = rc.Children.size();

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

		const EditorContext& context = EditorLayer::GetInstance()->GetContext();
		Entity selectedEntity = (context.IsValid(EditorContextType::Entity) ? *((Entity*)context.Data) : Entity({}));

		ImGuiTreeNodeFlags flags = (selectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0);
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth;
		flags |= ImGuiTreeNodeFlags_FramePadding;

		if (childrenSize == 0)
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		bool highlight = selectedEntity == entity;
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
		bool prefabColorApplied = isPartOfPrefab && entity != selectedEntity;
		if (prefabColorApplied)
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::HeaderSelectedColor);

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, "%s %s", ICON_MDI_CUBE_OUTLINE, tag.c_str());

		if (highlight)
			ImGui::PopStyleColor(2);

		// Select
		if (ImGui::IsItemDeactivated() && ImGui::IsItemHovered() && !ImGui::IsItemToggledOpen())
			EditorLayer::GetInstance()->SetContext(EditorContextType::Entity, &entity, sizeof(entity));

		// Expand recursively
		if (ImGui::IsItemToggledOpen() && (ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt)))
			forceExpandTree = opened;

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
				if (const ImGuiPayload* entityPayload = ImGui::AcceptDragDropPayload("Entity"))
				{
					m_DraggedEntity = *((Entity*)entityPayload->Data);
					m_DraggedEntityTarget = entity;
				}
				else if (const ImGuiPayload* assetPanelPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const char* path = (const char*)assetPanelPayload->Data;
					eastl::string ext = StringUtils::GetExtension(path);
					if (ext == "prefab")
					{
						m_DraggedEntity = EntitySerializer::DeserializeEntityAsPrefab(path, *m_Context);
						m_DraggedEntity = entity;
					}
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
			size_t minSize = glm::min(sizeof(buffer), tag.size());
			strncpy_s(buffer, tag.c_str(), minSize);
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				tag = eastl::string(buffer);

			if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
				renaming = {};
		}

		ImGui::TableNextColumn();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0 });

		float buttonSizeX = ImGui::GetContentRegionAvail().x;
		float frameHeight = ImGui::GetFrameHeight();
		ImGui::Button(isPartOfPrefab ? "Prefab" : "Entity", { buttonSizeX, frameHeight });
		// Select
		if (ImGui::IsItemDeactivated() && ImGui::IsItemHovered() && !ImGui::IsItemToggledOpen())
			EditorLayer::GetInstance()->SetContext(EditorContextType::Entity, &entity, sizeof(entity));

		ImGui::TableNextColumn();
		// Visibility Toggle
		{
			ImGui::Text("  %s", tagComponent.Enabled ? ICON_MDI_EYE_OUTLINE : ICON_MDI_EYE_OFF_OUTLINE);
			
			if (!ImGui::IsItemHovered())
				tagComponent.handled = false;

			if (ImGui::IsItemHovered() && ((!tagComponent.handled && ImGui::IsMouseDragging(0)) || ImGui::IsItemClicked()))
			{
				tagComponent.handled = true;
				tagComponent.Enabled = !tagComponent.Enabled;
			}
		}

		ImGui::PopStyleColor(3);

		if (prefabColorApplied)
			ImGui::PopStyleColor();

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
					default:
						treeLineColor = ImColor(255, 255, 255);
						break;
				}

				ImDrawList* drawList = ImGui::GetWindowDrawList();

				ImVec2 verticalLineEnd = verticalLineStart;

				for (size_t i = 0; i < childrenSize; i++)
				{
					UUID childId = entity.GetRelationship().Children[i];
					Entity child = m_Context->GetEntity(childId);
					const float HorizontalTreeLineSize = child.GetRelationship().Children.empty() ? 18.0f : 9.0f; //chosen arbitrarily
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
			m_DeletedEntity = entity;

		return nodeRect;
	}

	void SceneHierarchyPanel::DrawContextMenu() const
	{
		if (ImGui::BeginPopupContextWindow("SceneHierarchyContextWindow"))
		{
			EditorLayer::GetInstance()->ResetContext();
			Entity toSelect = {};
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Empty Entity"))
				{
					toSelect = m_Context->CreateEntity("Empty Entity");
				}
				else if (ImGui::MenuItem("Camera"))
				{
					toSelect = m_Context->CreateEntity("Camera");
					toSelect.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
				else if (ImGui::MenuItem("Sprite"))
				{
					toSelect = m_Context->CreateEntity("Sprite");
					toSelect.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
				else if (ImGui::MenuItem("Mesh"))
				{
					toSelect = m_Context->CreateEntity("Mesh");
					toSelect.AddComponent<MeshComponent>();
					ImGui::CloseCurrentPopup();
				}
				else if (ImGui::MenuItem("Sky Light"))
				{
					toSelect = m_Context->CreateEntity("Sky Light");
					toSelect.AddComponent<SkyLightComponent>();
					ImGui::CloseCurrentPopup();
				}
				else if (ImGui::MenuItem("Light"))
				{
					toSelect = m_Context->CreateEntity("Light");
					toSelect.AddComponent<LightComponent>();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (toSelect)
				EditorLayer::GetInstance()->SetContext(EditorContextType::Entity, &toSelect, sizeof(toSelect));

			ImGui::EndPopup();
		}
	}
}
