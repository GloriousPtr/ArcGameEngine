#include "SceneHierarchyPanel.h"

#include <Arc/Scene/EntitySerializer.h>

#include <Icons.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <string>

#include "../EditorLayer.h"
#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const char* name)
		: BasePanel(name, ARC_ICON_HIERARCHY, true)
	{
	}

	void SceneHierarchyPanel::OnUpdate([[maybe_unused]] Timestep ts)
	{
		const EditorContext& context = EditorLayer::GetInstance()->GetContext();
		m_SelectedEntity = (context.IsValid(EditorContextType::Entity) ? *context.As<Entity>() : Entity({}));

		if (ImGui::IsKeyPressed(ImGuiKey_F2))
		{
			m_RenamingEntity = m_SelectedEntity;
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
			const float lineHeight = ImGui::GetTextLineHeight();
			const ImVec2 padding = ImGui::GetStyle().FramePadding;

			const float filterCursorPosX = ImGui::GetCursorPosX();
			m_Filter.Draw("###HierarchyFilter", ImGui::GetContentRegionAvail().x - (UI::GetIconButtonSize("  " ARC_ICON_PLUS, "Add  ").x + 2.0f * padding.x));
			
			ImGui::SameLine();

			if (UI::IconButton("  " ARC_ICON_PLUS, "Add  "))
				ImGui::OpenPopup("SceneHierarchyContextWindow");

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);
			if (ImGui::BeginPopupContextWindow("SceneHierarchyContextWindow", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				DrawContextMenu();
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			if (!m_Filter.IsActive())
			{
				ImGui::SameLine();
				ImGui::SetCursorPosX(filterCursorPosX + ImGui::GetFontSize() * 0.5f);
				ImGui::TextUnformatted(ARC_ICON_SEARCH " Search...");
			}

			const ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 region = ImGui::GetContentRegionAvail();
			if (region.x == 0.0f)
				region.x = 1.0f;
			if (region.y == 0.0f)
				region.y = 1.0f;
			ImGui::InvisibleButton("##DragDropTargetBehindTable", region);
			DragDropTarget();

			ImGui::SetCursorPos(cursorPos);

			if (ImGui::BeginTable("HierarchyTable", 3, tableFlags))
			{
				ImGui::TableSetupColumn(" Label", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip);
				ImGui::TableSetupColumn(" Type", ImGuiTableColumnFlags_WidthFixed, lineHeight * 3.0f);
				ImGui::TableSetupColumn(" " ARC_ICON_EYE, ImGuiTableColumnFlags_WidthFixed, lineHeight * 2.0f);

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

				if (m_Context)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					const auto view = m_Context->m_Registry.view<IDComponent>();
					for (auto e = view.rbegin(); e != view.rend(); ++e)
					{
						const Entity entity = { *e, m_Context };
						if (entity && !entity.GetParent())
							DrawEntityNode(entity);
					}
					ImGui::PopStyleVar();
				}

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);
				if (ImGui::BeginPopupContextWindow("SceneHierarchyContextWindow", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
				{
					EditorLayer::GetInstance()->ResetContext();
					DrawContextMenu();
					ImGui::EndPopup();
				}
				ImGui::PopStyleVar();

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

			if (m_DeletedEntity && m_Context)
			{
				const EditorContext& context = EditorLayer::GetInstance()->GetContext();
				if (context.IsValid(EditorContextType::Entity) && *context.As<Entity>() == m_DeletedEntity)
					EditorLayer::GetInstance()->ResetContext();

				m_Context->DestroyEntity(m_DeletedEntity);
				m_DeletedEntity = {};
			}

			OnEnd();
		}
		ImGui::PopStyleVar();
	}

	ImRect SceneHierarchyPanel::DrawEntityNode(Entity entity, uint32_t depth, bool forceExpandTree, bool isPartOfPrefab)
	{
		ARC_PROFILE_SCOPE();

		[[unlikely]]
		if (!entity || !m_Context)
			return { 0, 0, 0, 0 };

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		auto& tagComponent = entity.GetComponent<TagComponent>();
		auto& tag = tagComponent.Tag;

		const auto& rc = entity.GetRelationship();
		const size_t childrenSize = rc.Children.size();

		if (m_Filter.IsActive() && !m_Filter.PassFilter(tag.c_str()))
		{
			for (const auto& child : rc.Children)
			{
				const UUID childId = entity.GetRelationship().Children[child];
				DrawEntityNode(m_Context->GetEntity(childId));
			}
			return { 0, 0, 0, 0 };
		}

		const bool selected = m_SelectedEntity == entity;
		ImGuiTreeNodeFlags flags = (selected ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_AllowOverlap;

		if (childrenSize == 0)
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		if (selected)
		{
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(EditorTheme::HeaderSelectedColor));
			ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::HeaderSelectedColor);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderSelectedColor);
		}

		if (forceExpandTree)
			ImGui::SetNextItemOpen(true);
		
		if (!isPartOfPrefab)
			isPartOfPrefab = entity.HasComponent<PrefabComponent>();
		const bool prefabColorApplied = isPartOfPrefab && entity != m_SelectedEntity;
		if (prefabColorApplied)
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::HeaderSelectedColor);

		const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(entity.GetUUID())), flags, "%s %s", ARC_ICON_ENTITY, tag.c_str());

		if (selected)
			ImGui::PopStyleColor(2);

		// Select
		if (!ImGui::IsItemToggledOpen() &&
			(ImGui::IsItemClicked(ImGuiMouseButton_Left) ||
				ImGui::IsItemClicked(ImGuiMouseButton_Middle) ||
				ImGui::IsItemClicked(ImGuiMouseButton_Right)))
		{
			EditorLayer::GetInstance()->SetContext(EditorContextType::Entity, reinterpret_cast<const char*>(&entity), sizeof(entity));
		}

		// Expand recursively
		if (ImGui::IsItemToggledOpen() && (ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt)))
			forceExpandTree = opened;

		bool entityDeleted = false;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);
		if (ImGui::BeginPopupContextItem())
		{
			if (m_SelectedEntity != entity)
				EditorLayer::GetInstance()->SetContext(EditorContextType::Entity, reinterpret_cast<const char*>(&entity), sizeof(entity));

			if (ImGui::MenuItemEx("Rename", ARC_ICON_RENAME, "F2"))
				m_RenamingEntity = entity;
			if (ImGui::MenuItemEx("Duplicate", ARC_ICON_DUPLICATE, "Ctrl+D"))
				m_Context->Duplicate(entity);
			if (ImGui::MenuItemEx("Delete", ARC_ICON_DELETE, "Del"))
				entityDeleted = true;

			ImGui::Separator();

			DrawContextMenu();
			
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		
		ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
		verticalLineStart.x -= 0.5f;
		verticalLineStart.y -= ImGui::GetFrameHeight() * 0.5f;

		// Drag Drop
		{
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* entityPayload = ImGui::AcceptDragDropPayload("Entity"))
				{
					m_DraggedEntity = *static_cast<Entity*>(entityPayload->Data);
					m_DraggedEntityTarget = entity;
				}
				else if (const ImGuiPayload* assetPanelPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const char* path = static_cast<char*>(assetPanelPayload->Data);
					const auto ext = StringUtils::GetExtension(path);
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
				ImGui::SetDragDropPayload("Entity", &entity, sizeof(entity));
				ImGui::TextUnformatted(tag.c_str());
				ImGui::EndDragDropSource();
			}
		}

		if (entity == m_RenamingEntity)
		{
			static bool renaming = false;
			if (!renaming)
			{
				renaming = true;
				ImGui::SetKeyboardFocusHere();
			}

			std::string tagBuffer = tag.c_str();
			if (ImGui::InputText("##Tag", &tagBuffer))
				tag = tagBuffer.c_str();

			if (ImGui::IsItemDeactivated())
			{
				renaming = false;
				m_RenamingEntity = {};
			}
		}

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(isPartOfPrefab ? "Prefab" : "Entity");
		ImGui::TableNextColumn();
		// Visibility Toggle
		{
			ImGui::Text("  %s", tagComponent.Enabled ? ARC_ICON_EYE : ARC_ICON_EYE_OFF);
			
			if (!ImGui::IsItemHovered())
				tagComponent.handled = false;

			if (ImGui::IsItemHovered() && ((!tagComponent.handled && ImGui::IsMouseDragging(0)) || ImGui::IsItemClicked()))
			{
				tagComponent.handled = true;
				tagComponent.Enabled = !tagComponent.Enabled;
			}
		}

		if (prefabColorApplied)
			ImGui::PopStyleColor();

		// Open
		const ImRect nodeRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		{
			if (opened && !entityDeleted)
			{
				static ImColor treeLineColor[3] = { ImColor(234, 102, 186), ImColor(101, 173, 229), ImColor(239, 184, 57) };
				depth %= sizeof(treeLineColor) / sizeof(ImColor);
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				ImVec2 verticalLineEnd = verticalLineStart;
				constexpr float lineThickness = 1.5f;

				for (const auto& childId : rc.Children)
				{
					Entity child = m_Context->GetEntity(childId);
					const float HorizontalTreeLineSize = child.GetRelationship().Children.empty() ? 18.0f : 9.0f; //chosen arbitrarily
					const ImRect childRect = DrawEntityNode(child, depth + 1, forceExpandTree, isPartOfPrefab);

					const float midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;
					drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + HorizontalTreeLineSize, midpoint), treeLineColor[depth], lineThickness);
					verticalLineEnd.y = midpoint;
				}

				drawList->AddLine(verticalLineStart, verticalLineEnd, treeLineColor[depth], lineThickness);
			}

			if (opened && childrenSize > 0)
				ImGui::TreePop();
		}

		// PostProcess Actions
		if(entityDeleted)
			m_DeletedEntity = entity;

		return nodeRect;
	}

	void SceneHierarchyPanel::SetContext(Scene* context)
	{
		ARC_PROFILE_SCOPE();

		m_Context = context;
	}

	void SceneHierarchyPanel::DragDropTarget() const
	{
		if (!m_Context)
			return;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = static_cast<char*>(payload->Data);
				const eastl::string name(StringUtils::GetName(path));
				const auto ext = StringUtils::GetExtension(path);

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
					m_Context->CreateEntity(name).AddComponent<SkyLightComponent>().Texture = AssetManager::GetTextureCube(path);
				}
				else if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
				{
					m_Context->CreateEntity(name).AddComponent<SpriteRendererComponent>().Texture = AssetManager::GetTexture2D(path);
				}
				else if (ext == "obj" || ext == "fbx")
				{
					const auto& mesh = AssetManager::GetMesh(path);
					const Entity parent = m_Context->CreateEntity(mesh->GetName());
					const size_t meshCount = mesh->GetSubmeshCount();
					if (meshCount == 1)
					{
						auto& meshComponent = parent.AddComponent<MeshComponent>();
						meshComponent.MeshGeometry = mesh;
						meshComponent.SubmeshIndex = 0;
					}
					else
					{
						for (size_t i = 0; i < meshCount; i++)
						{
							const auto& submesh = mesh->GetSubmesh(i);
							Entity entity = m_Context->CreateEntity(submesh.Name);
							entity.SetParent(parent);
							auto& meshComponent = entity.AddComponent<MeshComponent>();
							meshComponent.MeshGeometry = mesh;
							meshComponent.SubmeshIndex = i;
						}
					}
				}
			}

			ImGui::EndDragDropTarget();
		}
	}

	void SceneHierarchyPanel::DrawContextMenu() const
	{
		if (!m_Context)
			return;

		const bool hasContext = m_SelectedEntity;

		if (!hasContext)
			EditorLayer::GetInstance()->ResetContext();

		Entity toSelect = {};
		if (ImGui::MenuItemEx("Empty Entity", ARC_ICON_ENTITY))
		{
			toSelect = m_Context->CreateEntity("EmptyEntity");
		}
		if (ImGui::BeginMenu("2D"))
		{
			if (ImGui::MenuItemEx("Sprite", ARC_ICON_SPRITE_RENDERER))
			{
				toSelect = m_Context->CreateEntity("Sprite");
				toSelect.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("3D"))
		{
			if (ImGui::MenuItemEx("Mesh", ARC_ICON_MESH))
			{
				toSelect = m_Context->CreateEntity("Mesh");
				toSelect.AddComponent<MeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			// Primitives
			{
				const char* filepath = nullptr;
				if (ImGui::MenuItem("Cube"))
				{
					filepath = "Resources/Models/Primitives/Cube.obj";
				}
				if (ImGui::MenuItem("Sphere"))
				{
					filepath = "Resources/Models/Primitives/Sphere.obj";
				}
				if (ImGui::MenuItem("Plane"))
				{
					filepath = "Resources/Models/Primitives/Plane.obj";
				}
				if (ImGui::MenuItem("Quad"))
				{
					filepath = "Resources/Models/Primitives/Quad.obj";
				}
				if (ImGui::MenuItem("Torus"))
				{
					filepath = "Resources/Models/Primitives/Torus.obj";
				}
				if (filepath)
				{
					const auto& mesh = AssetManager::GetMesh(filepath);
					toSelect = m_Context->CreateEntity(mesh->GetName());
					auto& meshComponent = toSelect.AddComponent<MeshComponent>();
					meshComponent.MeshGeometry = mesh;
					meshComponent.SubmeshIndex = 0;
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Light"))
		{
			if (ImGui::MenuItemEx("Sky Light", ARC_ICON_SKYLIGHT))
			{
				toSelect = m_Context->CreateEntity("SkyLight");
				toSelect.AddComponent<SkyLightComponent>();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Directional Light"))
			{
				toSelect = m_Context->CreateEntity("DirectionalLight");
				toSelect.AddComponent<LightComponent>().Type = LightComponent::LightType::Directional;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItemEx("Point Light", ARC_ICON_LIGHT))
			{
				toSelect = m_Context->CreateEntity("PointLight");
				toSelect.AddComponent<LightComponent>().Type = LightComponent::LightType::Point;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Spot Light"))
			{
				toSelect = m_Context->CreateEntity("SpotLight");
				toSelect.AddComponent<LightComponent>().Type = LightComponent::LightType::Spot;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Audio"))
		{
			if (ImGui::MenuItemEx("Audio Source", ARC_ICON_AUDIO_SOURCE))
			{
				toSelect = m_Context->CreateEntity("AudioSource");
				toSelect.AddComponent<AudioSourceComponent>();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItemEx("Audio Listener", ARC_ICON_AUDIO_LISTENER))
			{
				toSelect = m_Context->CreateEntity("AudioListener");
				toSelect.AddComponent<AudioListenerComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndMenu();
		}
		if (ImGui::MenuItemEx("Particle System", ARC_ICON_PARTICLE_SYSTEM))
		{
			toSelect = m_Context->CreateEntity("ParticleSystem");
			toSelect.AddComponent<ParticleSystemComponent>();
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItemEx("Camera", ARC_ICON_CAMERA))
		{
			toSelect = m_Context->CreateEntity("Camera");
			toSelect.AddComponent<CameraComponent>();
			ImGui::CloseCurrentPopup();
		}

		if (hasContext && toSelect)
			toSelect.SetParent(m_SelectedEntity);

		if (toSelect)
			EditorLayer::GetInstance()->SetContext(EditorContextType::Entity, reinterpret_cast<const char*>(&toSelect), sizeof(toSelect));
	}
}
