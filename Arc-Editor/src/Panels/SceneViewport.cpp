#include "SceneViewport.h"

#include <imgui/imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include "Arc/Math/Math.h"

namespace ArcEngine
{
	static int s_ID = 0;
	SceneViewport::SceneViewport()
	{
		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);
		m_IDFrameBuffer = Framebuffer::Create(fbSpec);

		m_EditorCamera.SetViewportSize(1280, 720);

		m_ID = "Viewport" + s_ID;
		s_ID++;
	}

	SceneViewport::~SceneViewport()
	{
	}

	void SceneViewport::OnUpdate(Ref<Scene>& scene, Timestep timestep)
	{
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_IDFrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}
		else if (scene->IsViewportDirty())
		{
			scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_EditorCamera.OnUpdate(timestep);

		if (m_ViewportHovered)
		{
			m_MousePosition = *((glm::vec2*) &(ImGui::GetMousePos()));
			if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				glm::vec3 position = m_EditorCamera.GetPosition();
				const glm::vec3 forward = m_EditorCamera.GetForward();
				const glm::vec3 right = m_EditorCamera.GetRight();

				float yaw = m_EditorCamera.GetYaw();
				float pitch = m_EditorCamera.GetPitch();

				const glm::vec2 change = (m_MousePosition - m_LastMousePosition) * m_MouseSensitivity;

				yaw += change.x;
				pitch = glm::clamp(pitch - change.y, -89.9f, 89.9f);

				float movementSpeed = m_MovementSpeed * (ImGui::IsKeyDown(Key::LeftShift) ? 3.0f : 1.0f);

				if (ImGui::IsKeyDown(Key::W))
					position += forward * movementSpeed * timestep.GetSeconds();
				else if (ImGui::IsKeyDown(Key::S))
					position -= forward * movementSpeed * timestep.GetSeconds();
				if (ImGui::IsKeyDown(Key::D))
					position += right * movementSpeed * timestep.GetSeconds();
				else if (ImGui::IsKeyDown(Key::A))
					position -= right * movementSpeed * timestep.GetSeconds();

				m_EditorCamera.SetYaw(yaw);
				m_EditorCamera.SetPitch(pitch);
				m_EditorCamera.SetPosition(position);
			}
			m_LastMousePosition = m_MousePosition;
		}

		// Update scene
		scene->OnUpdateEditor(timestep, m_EditorCamera, m_Framebuffer);

		if (!ImGuizmo::IsUsing() && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			if (ImGui::IsKeyPressed(Key::Q))
				m_GizmoType = -1;
			if (ImGui::IsKeyPressed(Key::W))
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			if (ImGui::IsKeyPressed(Key::E))
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			if (ImGui::IsKeyPressed(Key::R))
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
			if (ImGui::IsKeyPressed(Key::T))
				m_GizmoType = ImGuizmo::OPERATION::BOUNDS;
		}

		if (ImGui::IsKeyPressed(Key::F) && m_SceneHierarchyPanel)
		{
			Entity entity = m_SceneHierarchyPanel->GetSelectedEntity();
			if (entity)
			{
				auto& transform = entity.GetComponent<TransformComponent>();
				glm::vec3 pos = transform.Translation;
				pos.z -= 5.0f;
				auto& view = glm::lookAt(pos, transform.Translation, m_EditorCamera.GetUp());
				m_EditorCamera.SetPosition(view[3]);
				m_EditorCamera.SetYaw(90);
				m_EditorCamera.SetPitch(0);
			}
		}


		if (ImGui::IsMouseClicked(0) && m_ViewportHovered && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
		{
			// Mouse Picking
			auto [mx, my] = ImGui::GetMousePos();
			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;
			const auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
			const auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;
			my = viewportHeight - my;
			const int mouseX = (int)mx;
			const int mouseY = (int)my;
			if(mouseX >= 0 && mouseY >= 0 && mouseX < viewportWidth && mouseY < viewportHeight)
			{
				int pixelData = scene->GetPixelDataAtPoint(mouseX, mouseY);
				ARC_ERROR(pixelData);
				const Entity selectedEntity = pixelData < 0 || !scene->HasEntity(pixelData) ? Entity() : scene->GetEntity(pixelData);
				m_SceneHierarchyPanel->SetSelectedEntity(selectedEntity);
			}
		}
	}

	void SceneViewport::OnImGuiRender()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin(m_ID.c_str());

		const auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		const auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		const auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		
		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
		
		const uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		// Gizmos
		if (m_ViewportHovered && m_SceneHierarchyPanel && m_GizmoType != -1)
		{
			Entity selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
			if(selectedEntity)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
				const glm::mat4& cameraView = m_EditorCamera.GetViewMatrix();
				// Entity Transform
				auto& tc = selectedEntity.GetComponent<TransformComponent>();
				glm::mat4 transform = selectedEntity.GetWorldTransform();

				// Snapping
				const bool snap = ImGui::IsKeyDown(Key::LeftControl);
				float snapValue = 0.5f;
				if(m_GizmoType == ImGuizmo::OPERATION::ROTATE)
					snapValue = 45.0f;

				float snapValues[3] = { snapValue, snapValue, snapValue };
				
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing())
				{
					glm::mat4& parentWorldTransform = tc.Parent != 0 ? selectedEntity.GetParent().GetWorldTransform() : glm::mat4(1.0f);
					glm::vec3 translation, rotation, scale;
					Math::DecomposeTransform(glm::inverse(parentWorldTransform) * transform, translation, rotation, scale);
					
					tc.Translation = translation;
					const glm::vec3 deltaRotation = rotation - tc.Rotation;
					tc.Rotation += deltaRotation;
					tc.Scale = scale;
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
