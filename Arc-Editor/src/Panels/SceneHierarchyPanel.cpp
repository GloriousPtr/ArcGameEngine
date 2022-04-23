#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ArcEngine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
		m_CurrentlyVisibleEntities = 0;
	}

	static void DrawRowsBackground(int row_count, float line_height, float x1, float x2, float y_offset, ImU32 col_even, ImU32 col_odd)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		float y0 = ImGui::GetCursorScreenPos().y + (float)(int)y_offset;

		int row_display_start;
		int row_display_end;
		ImGui::CalcListClipping(row_count, line_height, &row_display_start, &row_display_end);
		for (int row_n = row_display_start; row_n < row_display_end; row_n++)
		{
			ImU32 col = (row_n & 1) ? col_odd : col_even;
			if ((col & IM_COL32_A_MASK) == 0)
				continue;
			float y1 = y0 + (line_height * row_n);
			float y2 = y1 + line_height;
			draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col);
		}
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin("Scene Hierarchy");

		float x1 = ImGui::GetCurrentWindow()->WorkRect.Min.x;
		float x2 = ImGui::GetCurrentWindow()->WorkRect.Max.x;
		float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
		float item_offset_y = -item_spacing_y * 0.5f;
		float line_height = ImGui::GetTextLineHeight() + item_spacing_y;
		
		DrawRowsBackground(m_CurrentlyVisibleEntities, line_height, x1, x2, item_offset_y, 0, ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.20f, 1.0f)));
		m_CurrentlyVisibleEntities = 0;

		m_Context->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Context.get() };
			if (!entity.GetParent())
				DrawEntityNode(entity);
		});

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

	ImRect SceneHierarchyPanel::DrawEntityNode(Entity entity, bool skipChildren = false)
	{
		m_CurrentlyVisibleEntities++;

		auto& tagComponent = entity.GetComponent<TagComponent>();
		auto& tag = tagComponent.Tag;
		auto& transform = entity.GetTransform();
		uint32_t childrenSize = transform.Children.size();

		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		flags |= childrenSize == 0 ? ImGuiTreeNodeFlags_Bullet : 0;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, tag.c_str());

		if(ImGui::IsItemClicked())
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
			const ImColor TreeLineColor = ImGui::GetColorU32(ImGuiCol_Text);
			const float indentSize = 1.0f;
			const float SmallOffsetX = - (6.0f + indentSize);
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
			verticalLineStart.x += SmallOffsetX; //to nicely line up with the arrow symbol
			ImVec2 verticalLineEnd = verticalLineStart;

			ImGui::Indent(indentSize);
			for (size_t i = 0; i < childrenSize; i++)
			{
				UUID childId = entity.GetTransform().Children[i];
				const float HorizontalTreeLineSize = 8.0f; //chosen arbitrarily
	            const ImRect childRect = DrawEntityNode(m_Context->GetEntity(childId), createChild);
				const float midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;
			    drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + HorizontalTreeLineSize, midpoint), TreeLineColor);
				verticalLineEnd.y = midpoint;
			}
			ImGui::Unindent(indentSize);

			drawList->AddLine(verticalLineStart, verticalLineEnd, TreeLineColor);
		}

		if (opened)
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
