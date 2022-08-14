#include "SceneViewport.h"

#include <imgui/imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <entt.hpp>

#include "Arc/Math/Math.h"
#include "../Utils/IconsMaterialDesignIcons.h"
#include "../EditorLayer.h"
#include "../Utils/UI.h"

namespace ArcEngine
{
	static int s_ID = 0;
	
	SceneViewport::SceneViewport(const char* name)
		: BasePanel(name, ICON_MDI_TERRAIN, true)
	{
		OnInit();
	}

	void SceneViewport::OnInit()
	{
		ARC_PROFILE_SCOPE();

		m_RenderGraphData = CreateRef<RenderGraphData>();

		uint32_t width = 1280;
		uint32_t height = 720;
		{
			FramebufferSpecification spec;
			spec.Attachments = { FramebufferTextureFormat::R11G11B10 };
			spec.Width = width;
			spec.Height = height;
			m_RenderGraphData->CompositePassTarget = Framebuffer::Create(spec);
		}

		{
			FramebufferSpecification spec;
			spec.Attachments = {
				FramebufferTextureFormat::RGBA8,				// Albedo
				FramebufferTextureFormat::RG16F,				// Normal
				FramebufferTextureFormat::RGBA8,				// Metallic, Roughness, AO
				FramebufferTextureFormat::RGBA8,				// rgb: EmissionColor, a: intensity
				FramebufferTextureFormat::Depth
			};
			spec.Width = width;
			spec.Height = height;
			m_RenderGraphData->RenderPassTarget = Framebuffer::Create(spec);
		}

		{
			FramebufferSpecification spec;
			spec.Attachments = { FramebufferTextureFormat::R11G11B10 };
			spec.Width = width;
			spec.Height = height;
			m_RenderGraphData->LightingPassTarget = Framebuffer::Create(spec);
		}

		width /= 2;
		height /= 2;
		FramebufferSpecification bloomSpec;
		bloomSpec.Attachments = { FramebufferTextureFormat::R11G11B10 };
		bloomSpec.Width = width;
		bloomSpec.Height = height;
		m_RenderGraphData->PrefilteredFramebuffer = Framebuffer::Create(bloomSpec);
		
		for (size_t i = 0; i < m_RenderGraphData->BlurSamples; i++)
		{
			width /= 2;
			height /= 2;
			FramebufferSpecification blurSpec;
			blurSpec.Attachments = { FramebufferTextureFormat::R11G11B10 };
			blurSpec.Width = width;
			blurSpec.Height = height;
			m_RenderGraphData->TempBlurFramebuffers[i] = Framebuffer::Create(bloomSpec);
			m_RenderGraphData->DownsampledFramebuffers[i] = Framebuffer::Create(blurSpec);
			m_RenderGraphData->UpsampledFramebuffers[i] = Framebuffer::Create(blurSpec);
		}

		m_EditorCamera.SetViewportSize(1280, 720);
	}

	void SceneViewport::OnUpdate(Ref<Scene>& scene, Timestep timestep, bool useEditorCamera)
	{
		ARC_PROFILE_SCOPE(m_ID.c_str());

		m_Scene = scene;

		if (FramebufferSpecification spec = m_RenderGraphData->CompositePassTarget->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_RenderGraphData->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}
		else if (scene->IsViewportDirty())
		{
			scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if (!m_SimulationRunning && useEditorCamera)
		{
			m_MousePosition = *((glm::vec2*) &(ImGui::GetMousePos()));
			const glm::vec3& position = m_EditorCamera.GetPosition();
			float yaw = m_EditorCamera.GetYaw();
			float pitch = m_EditorCamera.GetPitch();

			bool moved = false;
			if (m_CursorLocked)
			{
				const glm::vec2 change = (m_MousePosition - m_LastMousePosition) * m_MouseSensitivity;
				yaw += change.x;
				pitch = glm::clamp(pitch - change.y, -89.9f, 89.9f);

				glm::vec3 moveDirection = glm::vec3(0.0f);
				if (ImGui::IsKeyDown(Key::W))
				{
					moved = true;
					moveDirection += m_EditorCamera.GetForward() * timestep.GetSeconds();
				}
				else if (ImGui::IsKeyDown(Key::S))
				{
					moved = true;
					moveDirection -= m_EditorCamera.GetForward() * timestep.GetSeconds();
				}
				if (ImGui::IsKeyDown(Key::D))
				{
					moved = true;
					moveDirection += m_EditorCamera.GetRight() * timestep.GetSeconds();
				}
				else if (ImGui::IsKeyDown(Key::A))
				{
					moved = true;
					moveDirection -= m_EditorCamera.GetRight() * timestep.GetSeconds();
				}

				if (glm::length2(moveDirection) > Math::EPSILON)
					m_MoveDirection = glm::normalize(moveDirection);
			}

			m_MoveVelocity += (moved ? 1.0f : -1.0f) * timestep;
			m_MoveVelocity *= glm::pow(m_MoveDampeningFactor, timestep);
			if (m_MoveVelocity > 0.0f)
			{
				float maxMoveSpeed = m_MaxMoveSpeed * (ImGui::IsKeyDown(Key::LeftShift) ? 3.0f : 1.0f);
				m_EditorCamera.SetPosition(position + (m_MoveDirection * m_MoveVelocity * maxMoveSpeed));
			}

			m_EditorCamera.SetYaw(yaw);
			m_EditorCamera.SetPitch(pitch);
			m_LastMousePosition = m_MousePosition;

			m_EditorCamera.OnUpdate(timestep);
		}

		// Update scene
		m_RenderGraphData->RenderPassTarget->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		if (m_SimulationRunning)
		{
			scene->OnUpdateRuntime(timestep, m_RenderGraphData, useEditorCamera ? &m_EditorCamera : nullptr);
			if (useEditorCamera)
				OnOverlayRender(scene);
		}
		else
		{
			scene->OnUpdateEditor(timestep, m_RenderGraphData, m_EditorCamera);
			OnOverlayRender(scene);
		}

		m_RenderGraphData->RenderPassTarget->Unbind();

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
			EditorContext context = EditorLayer::GetInstance()->GetContext();
			if (context.Type == EditorContextType::Entity && context.Data)
			{
				Entity entity = *((Entity*)context.Data);
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
				// TODO: Raycast selection
				/*
				const Entity selectedEntity = pixelData < 0 || !scene->HasEntity(pixelData) ? Entity() : scene->GetEntity(pixelData);
				m_SceneHierarchyPanel->SetSelectedEntity(selectedEntity);
				*/
			}
		}
	}

	void SceneViewport::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		if (OnBegin())
		{
			ImVec2 startCursorPos = ImGui::GetCursorPos();

			const auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			const auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			const auto viewportOffset = ImGui::GetWindowPos();
			m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
			m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();

			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			uint64_t textureID = m_RenderGraphData->CompositePassTarget->GetColorAttachmentRendererID(0);
			ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			if (m_SceneHierarchyPanel)
				m_SceneHierarchyPanel->DragDropTarget();

			// Gizmos
			if (m_ViewportHovered && m_SceneHierarchyPanel && m_GizmoType != -1 && !m_SimulationRunning)
			{
				EditorContext context = EditorLayer::GetInstance()->GetContext();
				if (context.Type == EditorContextType::Entity && context.Data)
				{
					Entity selectedEntity = *((Entity*)context.Data);
					if (selectedEntity)
					{
						ImGuizmo::SetOrthographic(false);
						ImGuizmo::SetDrawlist();

						ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

						const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
						const glm::mat4& cameraView = m_EditorCamera.GetViewMatrix();
						// Entity Transform
						auto& tc = selectedEntity.GetComponent<TransformComponent>();
						auto& rc = selectedEntity.GetComponent<RelationshipComponent>();
						glm::mat4 transform = selectedEntity.GetWorldTransform();

						// Snapping
						const bool snap = ImGui::IsKeyDown(Key::LeftControl);
						float snapValue = 0.5f;
						if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
							snapValue = 45.0f;

						float snapValues[3] = { snapValue, snapValue, snapValue };

						ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

						if (m_ViewportHovered && ImGuizmo::IsUsing())
						{
							glm::mat4& parentWorldTransform = rc.Parent != 0 ? selectedEntity.GetParent().GetWorldTransform() : glm::mat4(1.0f);
							glm::vec3 translation, rotation, scale;
							Math::DecomposeTransform(glm::inverse(parentWorldTransform) * transform, translation, rotation, scale);

							tc.Translation = translation;
							const glm::vec3 deltaRotation = rotation - tc.Rotation;
							tc.Rotation += deltaRotation;
							tc.Scale = scale;
						}
					}
				}
			}

			// Buttons
			ImGui::SetItemAllowOverlap();
			ImGui::SetCursorPos({ startCursorPos.x + windowPadding.x, startCursorPos.y + windowPadding.y });
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1, 1 });
			constexpr float alpha = 0.6f;
			if (UI::ToggleButton(ICON_MDI_ARROW_ALL, m_GizmoType == ImGuizmo::TRANSLATE, { 0, 0 }, alpha, alpha))
				m_GizmoType = ImGuizmo::TRANSLATE;
			ImGui::SameLine();
			if (UI::ToggleButton(ICON_MDI_ROTATE_3D, m_GizmoType == ImGuizmo::ROTATE, { 0, 0 }, alpha, alpha))
				m_GizmoType = ImGuizmo::ROTATE;
			ImGui::SameLine();
			if (UI::ToggleButton(ICON_MDI_ARROW_EXPAND_ALL, m_GizmoType == ImGuizmo::SCALE, { 0, 0 }, alpha, alpha))
				m_GizmoType = ImGuizmo::SCALE;
			ImGui::PopStyleVar();

			OnEnd();
		}

		ImGui::PopStyleVar();
	}

	void SceneViewport::OnOverlayRender(Ref<Scene>& scene)
	{
		Renderer2D::BeginScene(m_EditorCamera);
		{
			auto view = scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto& [tc, cam] = view.get<TransformComponent, CameraComponent>(entity);
				glm::mat4 transform = Entity(entity, scene.get()).GetWorldTransform();

				const auto inv = glm::inverse(cam.Camera.GetProjection() * glm::inverse(transform));
				eastl::vector<glm::vec3> frustumCorners;
				for (unsigned int x = 0; x < 2; ++x)
				{
					for (unsigned int y = 0; y < 2; ++y)
					{
						for (unsigned int z = 0; z < 2; ++z)
						{
							const glm::vec4 pt = 
							inv * glm::vec4(
							2.0f * x - 1.0f,
							2.0f * y - 1.0f,
							2.0f * z - 1.0f,
							1.0f);
							frustumCorners.push_back(glm::vec3(pt) / pt.w);
						}
					}
				}

				glm::vec4 color = glm::vec4(1.0f);
				Renderer2D::DrawLine(frustumCorners[0], frustumCorners[1], color);
				Renderer2D::DrawLine(frustumCorners[0], frustumCorners[2], color);
				Renderer2D::DrawLine(frustumCorners[0], frustumCorners[4], color);
				Renderer2D::DrawLine(frustumCorners[1], frustumCorners[3], color);
				Renderer2D::DrawLine(frustumCorners[1], frustumCorners[5], color);
				Renderer2D::DrawLine(frustumCorners[2], frustumCorners[3], color);
				Renderer2D::DrawLine(frustumCorners[2], frustumCorners[6], color);
				Renderer2D::DrawLine(frustumCorners[3], frustumCorners[7], color);
				Renderer2D::DrawLine(frustumCorners[4], frustumCorners[5], color);
				Renderer2D::DrawLine(frustumCorners[4], frustumCorners[6], color);
				Renderer2D::DrawLine(frustumCorners[5], frustumCorners[7], color);
				Renderer2D::DrawLine(frustumCorners[6], frustumCorners[7], color);
			}
		}

		{
			auto view = scene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
			for (auto entity : view)
			{
				auto& [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);
				Renderer2D::DrawRect(Entity(entity, scene.get()).GetWorldTransform(), { 0.2f, 0.8f, 0.2f, 1.0f });
			}
		}

		Renderer2D::EndScene(m_RenderGraphData);
	}

	bool SceneViewport::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::Button1 && m_ViewportHovered && !m_SimulationRunning)
		{
			Application::Get().GetWindow().HideCursor();
			m_CursorLocked = true;
			return true;
		}

		return false;
	}

	bool SceneViewport::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
	{
		if (m_CursorLocked == true)
		{
			Application::Get().GetWindow().ShowCursor();
			m_CursorLocked = false;
			m_MoveVelocity = 0;
			m_MoveDirection = glm::vec3(0.0f);

			return true;
		}

		return false;
	}
}
