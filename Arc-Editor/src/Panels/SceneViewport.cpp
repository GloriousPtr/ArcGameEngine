#include "SceneViewport.h"

#include <icons/IconsMaterialDesignIcons.h>
#include <imgui/imgui_internal.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include "../EditorLayer.h"
#include "../Utils/UI.h"

namespace ArcEngine
{
	SceneViewport::SceneViewport(const char* name)
		: BasePanel(name, ICON_MDI_TERRAIN, true)
	{
		OnInit();
	}

	void SceneViewport::OnInit()
	{
		ARC_PROFILE_SCOPE()

		constexpr uint32_t width = 1280;
		constexpr uint32_t height = 720;
		m_RenderGraphData = CreateRef<RenderGraphData>(width, height);
		m_MiniViewportRenderGraphData = CreateRef<RenderGraphData>(width, height);
		m_EditorCamera.SetViewportSize(width, height);
	}

	void SceneViewport::OnUpdate([[maybe_unused]] Timestep timestep)
	{
		ARC_PROFILE_SCOPE(m_ID.c_str())

		if (FramebufferSpecification spec = m_RenderGraphData->CompositePassTarget->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized fb is invalid
			(spec.Width != static_cast<uint32_t>(m_ViewportSize.x) || spec.Height != static_cast<uint32_t>(m_ViewportSize.y)))
		{
			auto miniViewportSize = m_ViewportSize * m_MiniViewportSizeMultiplier;
			m_MiniViewportRenderGraphData->Resize(static_cast<uint32_t>(miniViewportSize.x), static_cast<uint32_t>(miniViewportSize.y));

			uint32_t w = static_cast<uint32_t>(m_ViewportSize.x);
			uint32_t h = static_cast<uint32_t>(m_ViewportSize.y);
			m_RenderGraphData->Resize(w, h);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_Scene->OnViewportResize(w, h);
		}
		else if (m_Scene->IsViewportDirty())
		{
			m_Scene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		}

		if (!m_SimulationRunning && m_UseEditorCamera)
		{
			const glm::vec3& position = m_EditorCamera.GetPosition();
			const glm::vec2 yawPitch = glm::vec2(m_EditorCamera.GetYaw(), m_EditorCamera.GetPitch());
			glm::vec3 finalPosition = position;
			glm::vec2 finalYawPitch = yawPitch;

			if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && m_ViewportHovered)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
				glm::vec2 newMousePosition = Input::GetMousePosition();

				if (!m_UsingEditorCamera)
				{
					m_UsingEditorCamera = true;
					m_LockedMousePosition = newMousePosition;
				}

				Input::SetMousePosition(m_LockedMousePosition);

				const glm::vec2 change = (newMousePosition - m_LockedMousePosition) * m_MouseSensitivity;
				finalYawPitch.x += change.x;
				finalYawPitch.y = glm::clamp(finalYawPitch.y - change.y, -89.9f, 89.9f);

				float maxMoveSpeed = m_MaxMoveSpeed * (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 3.0f : 1.0f);
				if (ImGui::IsKeyDown(ImGuiKey_W))
					finalPosition += m_EditorCamera.GetForward() * maxMoveSpeed;
				else if (ImGui::IsKeyDown(ImGuiKey_S))
					finalPosition -= m_EditorCamera.GetForward() * maxMoveSpeed;
				if (ImGui::IsKeyDown(ImGuiKey_D))
					finalPosition += m_EditorCamera.GetRight() * maxMoveSpeed;
				else if (ImGui::IsKeyDown(ImGuiKey_A))
					finalPosition -= m_EditorCamera.GetRight() * maxMoveSpeed;
			}
			else
			{
				m_UsingEditorCamera = false;
			}

			glm::vec3 dampedPosition = Math::SmoothDamp(position, finalPosition, m_TranslationVelocity, m_TranslationDampening, 10000.0f, timestep);
			glm::vec2 dampedYawPitch = Math::SmoothDamp(yawPitch, finalYawPitch, m_RotationVelocity, m_RotationDampening, 1000.0f, timestep);
			m_EditorCamera.SetPosition(dampedPosition);
			m_EditorCamera.SetYaw(dampedYawPitch.x);
			m_EditorCamera.SetPitch(dampedYawPitch.y);
		}

		// Change transform gizmo
		if (GImGui->ActiveId == 0 && !ImGuizmo::IsUsing() && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Q))
				m_GizmoType = -1;
			if (ImGui::IsKeyPressed(ImGuiKey_W))
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_E))
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_R))
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
			if (ImGui::IsKeyPressed(ImGuiKey_T))
				m_GizmoType = ImGuizmo::OPERATION::BOUNDS;
			if (ImGui::IsKeyPressed(ImGuiKey_Y))
				m_GizmoType = ImGuizmo::OPERATION::UNIVERSAL;
		}

		// Entity picking
		if (ImGui::IsMouseClicked(0) && m_ViewportHovered && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
		{
			// Mouse Picking
			auto [mx, my] = ImGui::GetMousePos();
			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;
			const auto viewportWidth = static_cast<int>(m_ViewportBounds[1].x - m_ViewportBounds[0].x);
			const auto viewportHeight = static_cast<int>(m_ViewportBounds[1].y - m_ViewportBounds[0].y);
			my = static_cast<float>(viewportHeight) - my;
			const int mouseX = static_cast<int>(mx);
			const int mouseY = static_cast<int>(my);
			if(mouseX >= 0 && mouseY >= 0 && mouseX < viewportWidth && mouseY < viewportHeight)
			{
				// TODO: Scene GetEntity or use ray-cast for selection
			}
		}

		m_EditorCamera.OnUpdate(timestep);

		// Update scene
		m_RenderGraphData->RenderPassTarget->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		if (m_SimulationRunning)
		{
			m_Scene->OnUpdateRuntime(timestep, m_RenderGraphData, m_UseEditorCamera ? &m_EditorCamera : nullptr);
			if (m_UseEditorCamera)
				OnOverlayRender();
		}
		else
		{
			m_Scene->OnUpdateEditor(timestep, m_RenderGraphData, m_EditorCamera);
			OnOverlayRender();
		}

		m_RenderGraphData->RenderPassTarget->Unbind();
	}

	void SceneViewport::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE()

		ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		if (OnBegin(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
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
			ImGui::Image(reinterpret_cast<ImTextureID>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			if (m_SceneHierarchyPanel)
				m_SceneHierarchyPanel->DragDropTarget();

			if (!m_SimulationRunning)
			{
				// Transform Gizmos
				uint64_t id = reinterpret_cast<uint64_t>(&m_ID);
				ImGuizmo::SetID(static_cast<int>(id));
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				glm::mat4 cameraView = m_EditorCamera.GetView();
				glm::mat4 cameraProjection = m_EditorCamera.GetProjection();

				if (m_SceneHierarchyPanel && m_GizmoType != -1)
				{
					ARC_PROFILE_SCOPE("Transform Gizmos")

					EditorContext context = EditorLayer::GetInstance()->GetContext();
					if (context.IsValid(EditorContextType::Entity))
					{
						Entity selectedEntity = *context.As<Entity>();
						if (selectedEntity)
						{
							// Entity Transform
							auto& tc = selectedEntity.GetComponent<TransformComponent>();
							glm::mat4 transform = selectedEntity.GetWorldTransform();

							// Snapping
							const bool snap = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
							float snapValue = 0.5f;
							if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
								snapValue = 45.0f;

							float snapValues[3] = { snapValue, snapValue, snapValue };
							float bounds[6] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

							ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), static_cast<ImGuizmo::OPERATION>(m_GizmoType), static_cast<ImGuizmo::MODE>(m_GizmoMode),
								glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr, static_cast<ImGuizmo::OPERATION>(m_GizmoType) == ImGuizmo::OPERATION::BOUNDS ? bounds : nullptr, snap ? snapValues : nullptr);

							if (ImGuizmo::IsUsing())
							{
								Entity parent = selectedEntity.GetParent();
								const glm::mat4& parentWorldTransform = parent ? parent.GetWorldTransform() : glm::mat4(1.0f);
								glm::vec3 translation, rotation, scale;
								if (Math::DecomposeTransform(glm::inverse(parentWorldTransform) * transform, translation, rotation, scale))
								{
									tc.Translation = translation;
									const glm::vec3 deltaRotation = rotation - tc.Rotation;
									tc.Rotation += deltaRotation;
									tc.Scale = scale;
								}
							}
						}
					}
				}

				// Cube-view
				ImGuizmo::ViewManipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), ImGuizmo::OPERATION::ROTATE_SCREEN, ImGuizmo::MODE::WORLD, glm::value_ptr(m_CubeViewMatrix), 8.0f,
					ImVec2(m_ViewportBounds[1].x - 128, m_ViewportBounds[0].y), ImVec2(128, 128), 0x10101010);
				if (m_ViewportFocused)
				{
					const glm::mat4 inverted = glm::inverse(cameraView);
					const glm::vec3 direction = -glm::vec3(inverted[2]);
					float yaw = glm::atan(direction.z, direction.x);
					float pitch = glm::asin(direction.y);
					m_EditorCamera.SetPitch(pitch);
					m_EditorCamera.SetYaw(yaw);	
				}

				// Transform Gizmos Button Group
				float frameHeight = 1.3f * ImGui::GetFrameHeight();
				ImVec2 framePadding = ImGui::GetStyle().FramePadding;
				ImVec2 buttonSize = { frameHeight, frameHeight };
				float buttonCount = 6.0f;
				ImVec2 gizmoPosition = { m_ViewportBounds[0].x + m_GizmoPosition.x, m_ViewportBounds[0].y + m_GizmoPosition.y };
				ImRect bb(gizmoPosition.x, gizmoPosition.y, gizmoPosition.x + buttonSize.x + 8, gizmoPosition.y + (buttonSize.y + 2) * (buttonCount + 0.5f));
				ImVec4 frameColor = ImGui::GetStyleColorVec4(ImGuiCol_Tab);
				frameColor.w = 0.5f;
				ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(frameColor), false, ImGui::GetStyle().FrameRounding);
				glm::vec2 tempGizmoPosition = m_GizmoPosition;

				ImGui::SetCursorPos({ startCursorPos.x + tempGizmoPosition.x + framePadding.x, startCursorPos.y + tempGizmoPosition.y });
				ImGui::BeginGroup();
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1, 1 });

					ImVec2 draggerCursorPos = ImGui::GetCursorPos();
					ImGui::SetCursorPosX(draggerCursorPos.x + framePadding.x);
					ImGui::TextUnformatted(StringUtils::FromChar8T(ICON_MDI_DOTS_HORIZONTAL));
					ImVec2 draggerSize = ImGui::CalcTextSize(StringUtils::FromChar8T(ICON_MDI_DOTS_HORIZONTAL));
					draggerSize.x *= 2.0f;
					ImGui::SetCursorPos(draggerCursorPos);
					ImGui::InvisibleButton("GizmoDragger", draggerSize);
					static ImVec2 lastMousePosition = ImGui::GetMousePos();
					ImVec2 mousePos = ImGui::GetMousePos();
					if (ImGui::IsItemActive())
					{
						m_GizmoPosition.x += mousePos.x - lastMousePosition.x;
						m_GizmoPosition.y += mousePos.y - lastMousePosition.y;
					}
					lastMousePosition = mousePos;

					constexpr float alpha = 0.6f;
					if (UI::ToggleButton(StringUtils::FromChar8T(ICON_MDI_AXIS_ARROW), m_GizmoType == ImGuizmo::TRANSLATE, buttonSize, alpha, alpha))
						m_GizmoType = ImGuizmo::TRANSLATE;
					if (UI::ToggleButton(StringUtils::FromChar8T(ICON_MDI_ROTATE_3D), m_GizmoType == ImGuizmo::ROTATE, buttonSize, alpha, alpha))
						m_GizmoType = ImGuizmo::ROTATE;
					if (UI::ToggleButton(StringUtils::FromChar8T(ICON_MDI_ARROW_EXPAND), m_GizmoType == ImGuizmo::SCALE, buttonSize, alpha, alpha))
						m_GizmoType = ImGuizmo::SCALE;
					if (UI::ToggleButton(StringUtils::FromChar8T(ICON_MDI_VECTOR_SQUARE), m_GizmoType == ImGuizmo::BOUNDS, buttonSize, alpha, alpha))
						m_GizmoType = ImGuizmo::BOUNDS;
					if (UI::ToggleButton(StringUtils::FromChar8T(ICON_MDI_ARROW_EXPAND_ALL), m_GizmoType == ImGuizmo::UNIVERSAL, buttonSize, alpha, alpha))
						m_GizmoType = ImGuizmo::UNIVERSAL;
					if (UI::ToggleButton(m_GizmoMode == ImGuizmo::WORLD ? StringUtils::FromChar8T(ICON_MDI_EARTH) : StringUtils::FromChar8T(ICON_MDI_EARTH_OFF), m_GizmoMode == ImGuizmo::WORLD, buttonSize, alpha, alpha))
						m_GizmoMode = m_GizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

					ImGui::PopStyleVar(2);
				}
				ImGui::EndGroup();
			}

			// Showing mini camera viewport if selected entity has a CameraComponent
			ImVec2 endCursorPos = ImGui::GetContentRegionMax();
			EditorContext context = EditorLayer::GetInstance()->GetContext();
			if (!m_SimulationRunning && context.IsValid(EditorContextType::Entity))
			{
				ARC_PROFILE_SCOPE("MiniViewport")

				Entity selectedEntity = *context.As<Entity>();
				if (selectedEntity && selectedEntity.HasComponent<CameraComponent>())
				{
					const glm::mat4 view = glm::inverse(selectedEntity.GetWorldTransform());
					const glm::mat4 proj = selectedEntity.GetComponent<CameraComponent>().Camera.GetProjection();
					CameraData cameraData =
					{
						view,
						proj,
						proj * view,
						selectedEntity.GetTransform().Translation
					};

					m_MiniViewportRenderGraphData->RenderPassTarget->Bind();
					m_Scene->OnRender(m_MiniViewportRenderGraphData, cameraData);
					m_MiniViewportRenderGraphData->RenderPassTarget->Unbind();

					ImGui::SetItemAllowOverlap();
					ImVec2 miniViewportSize = { m_ViewportSize.x * m_MiniViewportSizeMultiplier, m_ViewportSize.y * m_MiniViewportSizeMultiplier };
					ImGui::SetCursorPos({ endCursorPos.x - miniViewportSize.x - windowPadding.x, endCursorPos.y - miniViewportSize.y - windowPadding.y });
					uint64_t textureId = m_MiniViewportRenderGraphData->CompositePassTarget->GetColorAttachmentRendererID(0);
					ImGui::Image(reinterpret_cast<ImTextureID>(textureId), ImVec2{ miniViewportSize.x, miniViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
				}
			}

			OnEnd();
		}

		ImGui::PopStyleVar();
	}

	void SceneViewport::OnOverlayRender() const
	{
		ARC_PROFILE_CATEGORY("Debug Rendering", Profile::Category::Debug)

		Renderer2D::BeginScene(m_EditorCamera.GetViewProjection());
		{
			constexpr glm::vec4 color = glm::vec4(1.0f);

			auto view = m_Scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
			for (auto &&[entityHandle, tc, cam] : view.each())
			{
				Entity entity = { entityHandle, m_Scene.get() };
				const auto inv = glm::inverse(cam.Camera.GetProjection() * glm::inverse(entity.GetWorldTransform()));
				glm::vec3 frustumCorners[8];
				int i = 0;
				for (int x = 0; x < 2; ++x)
				{
					for (int y = 0; y < 2; ++y)
					{
						for (int z = 0; z < 2; ++z)
						{
							const glm::vec4 pt = 
							inv * glm::vec4(
							2.0f * static_cast<float>(x) - 1.0f,
							2.0f * static_cast<float>(y) - 1.0f,
							2.0f * static_cast<float>(z) - 1.0f,
							1.0f);
							frustumCorners[i] = (glm::vec3(pt) / pt.w);
							++i;
						}
					}
				}

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
			constexpr glm::vec4 color = { 0.32f, 0.53f, 0.78f, 1.0f };

			auto boxColliderView = m_Scene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
			for (auto &&[entity, tc, bc] : boxColliderView.each())
			{
				glm::mat4 transform = Entity(entity, m_Scene.get()).GetWorldTransform();
				transform *= glm::translate(glm::mat4(1.0f), glm::vec3(bc.Offset, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f * bc.Size, 1.0f));
				Renderer2D::DrawRect(transform, color);
			}

			auto polygonColliderView = m_Scene->GetAllEntitiesWith<TransformComponent, PolygonCollider2DComponent>();
			for (auto &&[entity, tc, pc] : polygonColliderView.each())
			{
				auto transform = Entity(entity, m_Scene.get()).GetWorldTransform();
				auto translation = glm::vec3(0.0f);
				auto rotation = glm::vec3(0.0f);
				auto scale = glm::vec3(0.0f);
				Math::DecomposeTransform(transform, translation, rotation, scale);
				transform = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(glm::quat(rotation));

				transform *= glm::translate(glm::mat4(1.0f), glm::vec3(pc.Offset, 0.0f));

				for (size_t i = 0; i < pc.Points.size(); ++i)
				{
					glm::vec4 p0 = transform * glm::vec4(pc.Offset + pc.Points[i], 0.0f, 1.0f);
					auto p1 = glm::vec4(0.0f);
					if (i == pc.Points.size() - 1)
						p1 = transform * glm::vec4(pc.Offset + pc.Points[0], 0.0f, 1.0f);
					else
						p1 = transform * glm::vec4(pc.Offset + pc.Points[i + 1], 0.0f, 1.0f);

					Renderer2D::DrawLine(p0, p1, color);
				}
			}
		}

		{
			constexpr glm::vec4 color = { 0.2f, 0.8f, 0.2f, 1.0f };

			auto view = m_Scene->GetAllEntitiesWith<TransformComponent, BoxColliderComponent>();
			for (auto &&[entity, tc, bc] : view.each())
			{
				glm::mat4 transform = Entity(entity, m_Scene.get()).GetWorldTransform();
				transform *= glm::translate(glm::mat4(1.0f), bc.Offset) * glm::scale(glm::mat4(1.0f), 4.0f * bc.Size);

				glm::mat4 transformFront = glm::translate(transform, glm::vec3(0.0f, 0.0f, bc.Size.z));
				glm::mat4 transformBack = glm::translate(transform, glm::vec3(0.0f, 0.0f, -bc.Size.z));
				glm::mat4 transformLeft = glm::translate(transform, glm::vec3(-bc.Size.x, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 transformRight = glm::translate(transformLeft, glm::vec3(0.0f, 0.0f, 2.0f * bc.Size.x));

				Renderer2D::DrawRect(transformFront, color);
				Renderer2D::DrawRect(transformBack, color);
				Renderer2D::DrawRect(transformLeft, color);
				Renderer2D::DrawRect(transformRight, color);
			}
		}

		Renderer2D::EndScene(m_RenderGraphData);
	}
}
