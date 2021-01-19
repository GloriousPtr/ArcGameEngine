﻿#include "SceneHierarchyPanel.h"

#include <iostream>


#include "Arc/Utils/PlatformUtils.h"

#include <glm/gtc/type_ptr.hpp>

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
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Context.get() };
			DrawEntityNode(entity);
		});

		if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		if(ImGui::BeginPopupContextWindow(0, 1, false))
		{
			m_SelectionContext = {};
			if(ImGui::MenuItem("Create Empty Entity"))
			{
				m_SelectionContext = m_Context->CreateEntity("Empty Entity");
			}
			else if (ImGui::MenuItem("Create Camera"))
			{
				m_SelectionContext = m_Context->CreateEntity("Camera");
				m_SelectionContext.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}
			else if (ImGui::MenuItem("Create Sprite"))
			{
				m_SelectionContext = m_Context->CreateEntity("Sprite");
				m_SelectionContext.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::EndPopup();
		}
		
		ImGui::End();
		

		
		ImGui::Begin("Properties");
		if(m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tagComponent = entity.GetComponent<TagComponent>();
		auto& tag = tagComponent.Tag;

		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if(ImGui::IsItemClicked())
			m_SelectionContext = entity;

		bool entityDeleted = false;
		if(ImGui::BeginPopupContextItem())
		{
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
		
		if(opened)
		{
			ImGui::TreePop();
		}

		if(entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if(m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	static void DrawFloatControl(const std::string& label, float* value, float min = 0, float max = 0, float columnWidth = 100.0f)
	{
		ImGui::PushID(label.c_str());
		
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::DragFloat("##value", value, 0.1f, min, max);

		ImGui::Columns(1);
		ImGui::PopID();
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		
		ImGui::PushID(label.c_str());
		
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if(ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if(ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if(ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		
		ImGui::Columns(1);

		ImGui::PopID();
	}

	static void DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f, const char* format = "%.2f", float columnWidth = 200.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0, 0, format);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0, 0, format);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, const bool removable = true)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		
		if(entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if(removable)
			{
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if(ImGui::Button("V", ImVec2{ lineHeight, lineHeight }))
					ImGui::OpenPopup("ComponentSettings");

				if(ImGui::BeginPopup("ComponentSettings"))
				{
					if(ImGui::MenuItem("Remove Component"))
						removeComponent = true;
					
					ImGui::EndPopup();
				}
			}
			if(open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if(removeComponent)
				entity.RemoveComponent<T>();
		}
	}
	
	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if(entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, tag.c_str());
			if(ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				tag = std::string(buffer);
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("Add Component");

		if (ImGui::BeginPopup("Add Component"))
		{
			if (ImGui::MenuItem("Camera"))
			{
				if (!entity.HasComponent<CameraComponent>())
					entity.AddComponent<CameraComponent>();
				else
					ARC_CORE_WARN("This entity already has the Camera Component!");
				ImGui::CloseCurrentPopup();
			}
			
			if (ImGui::MenuItem("Sprite Renderer"))
			{
				if (!entity.HasComponent<SpriteRendererComponent>())
					entity.AddComponent<SpriteRendererComponent>();
				else
					ARC_CORE_WARN("This entity already has the Sprite Renderer Component!");
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Rigidbody 2D"))
			{
				if (!entity.HasComponent<Rigidbody2DComponent>())
					entity.AddComponent<Rigidbody2DComponent>();
				else
					ARC_CORE_WARN("This entity already has the Rigidbody 2D Component!");
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Box Collider 2D"))
			{
				if (!entity.HasComponent<BoxCollider2DComponent>())
					entity.AddComponent<BoxCollider2DComponent>();
				else
					ARC_CORE_WARN("This entity already has the Box Collider 2D Component!");
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();
		
		DrawComponent<TransformComponent>("Transform", entity, [](TransformComponent& component)
		{
			DrawVec3Control("Translation", component.Translation);
			
			glm::vec3 rotation = glm::degrees(component.Rotation);
			DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);

			DrawVec3Control("Scale", component.Scale, 1.0f);
		}, false);

		DrawComponent<CameraComponent>("Camera", entity, [](CameraComponent& component)
		{
			auto& camera = component.Camera;

			ImGui::Checkbox("Primary", &component.Primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if(ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if(ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if(isSelected)
						ImGui::SetItemDefaultFocus();
				}
				
				ImGui::EndCombo();
			}

			if(camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if(ImGui::DragFloat("Vertical FOV", &verticalFov, 0.1f))
					camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));

				float perspectiveNear = camera.GetPerspectiveNearClip();
				if(ImGui::DragFloat("Near Clip", &perspectiveNear, 0.1f))
					camera.SetPerspectiveNearClip(perspectiveNear);
				
				float perspectiveFar = camera.GetPerspectiveFarClip();
				if(ImGui::DragFloat("Far Clip", &perspectiveFar, 0.1f))
					camera.SetPerspectiveFarClip(perspectiveFar);
			}

			if(camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if(ImGui::DragFloat("Size", &orthoSize, 0.1f))
					camera.SetOrthographicSize(orthoSize);

				float orthoNear = camera.GetOrthographicNearClip();
				if(ImGui::DragFloat("Near Clip", &orthoNear, 0.1f))
					camera.SetOrthographicNearClip(orthoNear);
				
				float orthoFar = camera.GetOrthographicFarClip();
				if(ImGui::DragFloat("Far Clip", &orthoFar, 0.1f))
					camera.SetOrthographicFarClip(orthoFar);

				ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
			}
		});
		
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

			const uint32_t id = component.Texture == nullptr ? 0 : component.Texture->GetRendererID();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
			
			ImGui::Text("Texture");
			const ImVec2 buttonSize = { 80, 80 };
			ImGui::SameLine(ImGui::GetWindowWidth() * 0.6f);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
			if(ImGui::ImageButton((ImTextureID)id, buttonSize, { 1, 1 }, { 0, 0}, 0))
			{
				std::string filepath = FileDialogs::OpenFile("Texture (*.png)\0*.png\0");
				if (!filepath.empty())
					component.SetTexture(filepath);
			}
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
			if(ImGui::Button("x", { buttonSize.x / 4, buttonSize.y } ))
				component.RemoveTexture();
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::Spacing();
			
			DrawFloatControl("Tiling Factor", &component.TilingFactor, 0, 0, 200);
		});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](Rigidbody2DComponent& component)
		{

			{
				const char* items[] = { "Static", "Kinematic", "Dynamic" };
				const char* current_item = items[(int)component.Specification.Type];
				ImGui::Text("Body Type");
				ImGui::SameLine();
				if (ImGui::BeginCombo("##BodyType", current_item))
				{
					for (int n = 0; n < 3; n++)
					{
						bool is_selected = (current_item == items[n]);
						if (ImGui::Selectable(items[n], is_selected))
						{
							current_item = items[n];
							component.Specification.Type = (Rigidbody2dType)n;
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			if (component.Specification.Type == Rigidbody2dType::Dynamic)
			{
				DrawFloatControl("Linear Damping", &(component.Specification.LinearDamping), 0.0f, 1000000.0f, 200);
				DrawFloatControl("Angular Damping", &(component.Specification.AngularDamping), 0.0f, 1000000.0f, 200);
				DrawFloatControl("Gravity Scale", &(component.Specification.GravityScale), -1000000.0f, 1000000.0f, 200);
			}
			if (component.Specification.Type == Rigidbody2dType::Dynamic || component.Specification.Type == Rigidbody2dType::Kinematic)
			{
				{
					const char* items[] = { "Discrete", "Continuous" };
					const char* current_item = items[(int)component.Specification.CollisionDetection];
					ImGui::Text("Collision Detection");
					ImGui::SameLine();
					if (ImGui::BeginCombo("##CollisionDetection", current_item))
					{
						for (int n = 0; n < 2; n++)
						{
							bool is_selected = (current_item == items[n]);
							if (ImGui::Selectable(items[n], is_selected))
							{
								current_item = items[n];
								component.Specification.CollisionDetection = (Rigidbody2D::CollisionDetectionType)n;
							}

							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
				}

				{
					const char* items[] = { "NeverSleep", "StartAwake", "StartAsleep" };
					const char* current_item = items[(int)component.Specification.SleepingMode];
					ImGui::Text("Sleeping Mode");
					ImGui::SameLine();
					if (ImGui::BeginCombo("##SleepingMode", current_item))
					{
						for (int n = 0; n < 3; n++)
						{
							bool is_selected = (current_item == items[n]);
							if (ImGui::Selectable(items[n], is_selected))
							{
								current_item = items[n];
								component.Specification.SleepingMode = (Rigidbody2D::SleepType)n;
							}

							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
				}

				ImGui::Text("Freeze Rotation");
				ImGui::SameLine();
				ImGui::Checkbox("##FreezeRotationZ", &component.Specification.FreezeRotationZ);
				ImGui::SameLine();
				ImGui::Text("Z");
			}

			component.ValidateSpecification();
		});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](BoxCollider2DComponent& component)
		{
			ImGui::Text("Is Trigger");
			ImGui::SameLine();
			ImGui::Checkbox("##IsTrigger", &component.IsTrigger);

			glm::vec2 size = component.Size;
			DrawVec2Control("Size", size, 1.0f, "%.4f");
			if (size.x <= 0.1f)
				size.x = 0.1f;
			if (size.y <= 0.1f)
				size.y = 0.1f;
			component.Size = size;

			DrawVec2Control("Offset", component.Offset, 0.0f, "%.4f");

			component.ValidateSpecification();
		});
	}
}
