#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace ArcEngine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		OPTICK_EVENT();

		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		OPTICK_EVENT();

		m_Context = context;
		m_SelectionContext = {};
		m_CurrentlyVisibleEntities = 0;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		OPTICK_EVENT();

		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin(ICON_MDI_VIEW_LIST " Hierarchy");

		float x1 = ImGui::GetCurrentWindow()->WorkRect.Min.x;
		float x2 = ImGui::GetCurrentWindow()->WorkRect.Max.x;
		float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
		float item_offset_y = -item_spacing_y * 0.5f;
		float line_height = ImGui::GetTextLineHeight() + item_spacing_y;
		UI::DrawRowsBackground(m_CurrentlyVisibleEntities, line_height, x1, x2, item_offset_y, 0, ImGui::GetColorU32(EditorTheme::WindowBgAlternativeColor));
		m_CurrentlyVisibleEntities = 0;
		
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

		if (m_DeletedEntity)
		{
			if(m_SelectionContext == m_DeletedEntity)
				m_SelectionContext = {};

			m_Context->DestroyEntity(m_DeletedEntity);
			m_DeletedEntity = {};
		}

		if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		if(ImGui::BeginPopupContextWindow(0, 1, false))
		{
			m_SelectionContext = {};
			if (ImGui::BeginMenu("Create"))
			{
				if(ImGui::MenuItem("Empty Entity"))
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

		ImGui::End();
	}

	ImRect SceneHierarchyPanel::DrawEntityNode(Entity entity, bool skipChildren)
	{
		OPTICK_EVENT();

		m_CurrentlyVisibleEntities++;

		auto& tagComponent = entity.GetComponent<TagComponent>();
		auto& tag = tagComponent.Tag;
		auto& rc = entity.GetRelationship();
		uint32_t childrenSize = rc.Children.size();

		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0);
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		if (childrenSize == 0)
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		bool highlight = m_SelectionContext == entity;
		if (highlight)
			ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::HeaderSelectedColor);

		std::string displayName = (ICON_MDI_CUBE_OUTLINE + std::string(" ") + tag);
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, displayName.c_str());

		if (highlight)
			ImGui::PopStyleColor();

		if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			m_SelectionContext = entity;

		bool entityDeleted = false;
		bool createChild = false;
		if(ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Create"))
				createChild = true;
			if (ImGui::MenuItem("Rename"))
				tagComponent.renaming = true;
			if(ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (tagComponent.renaming)
		{
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				tag = std::string(buffer);

			if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
				tagComponent.renaming = false;
		}

		const ImRect nodeRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		if(opened && !skipChildren && !entityDeleted)
		{
			const ImColor TreeLineColor = ImColor(128, 128, 128, 128);
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
			ImVec2 verticalLineEnd = verticalLineStart;

			for (size_t i = 0; i < childrenSize; i++)
			{
				UUID childId = entity.GetRelationship().Children[i];
				Entity child = m_Context->GetEntity(childId);
				const float HorizontalTreeLineSize = child.GetRelationship().Children.size() == 0 ? 18.0f : 10.0f; //chosen arbitrarily
	            const ImRect childRect = DrawEntityNode(child, createChild);
				const float midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;
			    drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + HorizontalTreeLineSize, midpoint), TreeLineColor, 0.25f);
				verticalLineEnd.y = midpoint;
			}

			drawList->AddLine(verticalLineStart, verticalLineEnd, TreeLineColor);
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
