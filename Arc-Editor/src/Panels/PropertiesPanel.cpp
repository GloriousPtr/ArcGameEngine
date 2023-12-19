#include "PropertiesPanel.h"

#include <icons/IconsMaterialDesignIcons.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <Arc/Scene/EntitySerializer.h>

#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace ArcEngine
{
	PropertiesPanel::PropertiesPanel(const char* name)
			: BasePanel(name, ICON_MDI_INFORMATION, true)
	{
	}

	void PropertiesPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		if (OnBegin())
		{
			if (m_Context)
			{
				switch (m_Context.GetType())
				{
					case EditorContextType::None:
						break;
					case EditorContextType::Entity:
					{
						const Entity selectedEntity = *m_Context.As<Entity>();
						if (selectedEntity && selectedEntity.GetScene())
							DrawComponents(selectedEntity);
						else if (m_Locked)
							m_Locked = false;
						break;
					}
					case EditorContextType::File:
					{
						DrawFileProperties(m_Context.As<char>());
						break;
					}
				}
			}
			OnEnd();
		}
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const char* name, Entity entity, UIFunction uiFunction, const bool removable = true)
	{
		ARC_PROFILE_SCOPE();

		if(entity.HasComponent<T>())
		{
			static constexpr ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_FramePadding;

			auto& component = entity.GetComponent<T>();
			
			const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + lineHeight * 0.25f);

			const size_t id = entt::type_id<T>().hash();
			const bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(id), treeFlags, "%s", reinterpret_cast<const char*>(name));

			bool removeComponent = false;
			if(removable)
			{
				ImGui::PushID(static_cast<int>(id));

				const float frameHeight = ImGui::GetFrameHeight();
				ImGui::SameLine(ImGui::GetContentRegionMax().x - frameHeight * 1.2f);
				if(ImGui::Button(ICON_MDI_SETTINGS, ImVec2{ frameHeight * 1.2f, frameHeight }))
					ImGui::OpenPopup("ComponentSettings");

				if(ImGui::BeginPopup("ComponentSettings"))
				{
					if(ImGui::MenuItem("Remove Component"))
						removeComponent = true;
					
					ImGui::EndPopup();
				}

				ImGui::PopID();
			}

			if(open)
			{
				ARC_PROFILE_SCOPE("UI Function");

				uiFunction(component);
				ImGui::TreePop();
			}

			if(removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	static void DrawFields(Entity entity, ScriptComponent& component)
	{
		static constexpr ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_FramePadding;

		eastl::string* toRemove = nullptr;
		
		const float frameHeight = ImGui::GetFrameHeight();
		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		for (auto it = component.Classes.begin(); it != component.Classes.end(); ++it)
		{
			const eastl::string& className = *it;

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + lineHeight * 0.25f);
			const bool open = ImGui::TreeNodeEx(&it, treeFlags, "%s", className.c_str());

			{
				ImGui::PushID(&it);

				ImGui::SameLine(ImGui::GetContentRegionMax().x - frameHeight * 1.2f);
				if (ImGui::Button(ICON_MDI_SETTINGS, ImVec2{ frameHeight * 1.2f, frameHeight }))
					ImGui::OpenPopup("ScriptSettings");

				if (ImGui::BeginPopup("ScriptSettings"))
				{
					if (ImGui::MenuItem("Remove"))
						toRemove = it;

					ImGui::EndPopup();
				}

				ImGui::PopID();
			}

			if (open)
			{
				ARC_PROFILE_SCOPE("UI Function");

				// Public Fields
				if (ScriptEngine::HasClass(className))
				{
					UI::BeginProperties();
					const auto& fields = ScriptEngine::GetFields(className.c_str());
					auto& fieldMap = ScriptEngine::GetFieldMap(className.c_str());
					for (const auto& name : fields)
					{
						auto& field = fieldMap.at(name);
						if (field.Hidden)
							continue;

						if (!field.Header.empty())
						{
							UI::EndProperties();
							ImGui::Spacing();
							ImGui::Spacing();
							ImGui::TextUnformatted(field.Header.c_str());
							ImGui::Spacing();
							UI::BeginProperties();
						}

						UI::DrawField(entity, className, name);
					}
					UI::EndProperties();
				}

				ImGui::TreePop();
			}
		}

		if (toRemove)
			component.Classes.erase(toRemove);
	}

	static void DrawMaterialProperties(const Ref<Material>& material)
	{
		UI::BeginProperties();

		const auto& materialProperties = material->GetProperties();
		for (const auto& property : materialProperties)
		{
			const eastl::string_view name = property.Name;
			const char* displayName = property.DisplayName.c_str();

			switch (property.Type)
			{
				case MaterialPropertyType::None: break;
				case MaterialPropertyType::Texture2DBindless:
				case MaterialPropertyType::Texture2D:
				{
					Ref<Texture2D> tex = material->GetTexture(name);
					if (UI::Property(displayName, tex, tex ? tex->GetRendererID() : 0))
						material->SetTexture(name, tex);
					break;
				}
				case MaterialPropertyType::Bool:
				{
					bool v = material->GetData<int32_t>(name) == 0 ? false : true;
					if (UI::Property(displayName, v))
						material->SetData(name, v ? 1 : 0);
					break;
				}
				case MaterialPropertyType::Int:
				{
					auto v = material->GetData<int32_t>(name);
					if (UI::Property(displayName, v))
						material->SetData(name, v);
					break;
				}
				case MaterialPropertyType::UInt:
				{
					auto v = material->GetData<uint32_t>(name);
					if (UI::Property(displayName, v))
						material->SetData(name, v);
					break;
				}
				case MaterialPropertyType::Float:
				{
					auto v = material->GetData<float>(name);
					if (name.ends_with("01"))
					{
						if (UI::Property(displayName, v, 0.0f, 1.0f))
							material->SetData(name, v);
					}
					else
					{
						if (UI::Property(displayName, v))
							material->SetData(name, v);
					}
					break;
				}
				case MaterialPropertyType::Float2:
				{
					auto v = material->GetData<glm::vec2>(name);
					if (UI::PropertyVector(displayName, v))
						material->SetData(name, v);
					break;
				}
				case MaterialPropertyType::Float3:
				{
					auto v = material->GetData<glm::vec3>(name);
					if (UI::PropertyVector(displayName, v, property.IsColor))
						material->SetData(name, v);
					break;
				}
				case MaterialPropertyType::Float4:
				{
					auto v = material->GetData<glm::vec4>(name);
					if (UI::PropertyVector(displayName, v, property.IsColor))
						material->SetData(name, v);
					break;
				}
			}
		}

		UI::EndProperties();
	}

	template<typename T>
	static void DrawParticleOverLifetimeModule(eastl::string_view moduleName, OverLifetimeModule<T>& propertyModule, bool color = false, bool rotation = false)
	{
		static constexpr ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_FramePadding;

		if (ImGui::TreeNodeEx(moduleName.data(), treeFlags, "%s", moduleName.data()))
		{
			UI::BeginProperties();
			UI::Property("Enabled", propertyModule.Enabled);

			if (rotation)
			{
				T degrees = glm::degrees(propertyModule.Start);
				if (UI::PropertyVector("Start", degrees))
					propertyModule.Start = glm::radians(degrees);

				degrees = glm::degrees(propertyModule.End);
				if (UI::PropertyVector("End", degrees))
					propertyModule.End = glm::radians(degrees);
			}
			else
			{
				UI::PropertyVector("Start", propertyModule.Start, color);
				UI::PropertyVector("End", propertyModule.End, color);
			}

			UI::EndProperties();

			ImGui::TreePop();
		}
	}

	template<typename T>
	static void DrawParticleBySpeedModule(eastl::string_view moduleName, BySpeedModule<T>& propertyModule, bool color = false, bool rotation = false)
	{
		static constexpr ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_FramePadding;

		if (ImGui::TreeNodeEx(moduleName.data(), treeFlags, "%s", moduleName.data()))
		{
			UI::BeginProperties();
			UI::Property("Enabled", propertyModule.Enabled);

			if (rotation)
			{
				T degrees = glm::degrees(propertyModule.Start);
				if (UI::PropertyVector("Start", degrees))
					propertyModule.Start = glm::radians(degrees);

				degrees = glm::degrees(propertyModule.End);
				if (UI::PropertyVector("End", degrees))
					propertyModule.End = glm::radians(degrees);
			}
			else
			{
				UI::PropertyVector("Start", propertyModule.Start, color);
				UI::PropertyVector("End", propertyModule.End, color);
			}

			UI::Property("Min Speed", propertyModule.MinSpeed);
			UI::Property("Max Speed", propertyModule.MaxSpeed);
			UI::EndProperties();
			ImGui::TreePop();
		}
	}

	void PropertiesPanel::DrawComponents(Entity entity)
	{
		ARC_PROFILE_SCOPE();

		const ImVec2 framePadding = ImGui::GetStyle().FramePadding;
		const float frameHeight = ImGui::GetFrameHeight();

		{
			const float regionX = ImGui::GetContentRegionAvail().x;
			const float addButtonSizeX = UI::GetIconButtonSize("  " ICON_MDI_PLUS, "Add  ").x;
			const ImVec2 lockButtonSize = ImVec2(frameHeight * 1.5f, frameHeight);
			const float tagWidth = regionX - ((addButtonSizeX + framePadding.x * 2.0f) + (lockButtonSize.x + framePadding.x * 2.0f));

			auto& tag = entity.GetComponent<TagComponent>();

			if (entity.HasComponent<TagComponent>())
			{
				ImGui::SetNextItemWidth(tagWidth);
				constexpr size_t bufferSize = 256;
				char buffer[bufferSize];
				memcpy_s(buffer, bufferSize, tag.Tag.c_str(), tag.Tag.size() + 1);
				if (ImGui::InputText("##Tag", buffer, bufferSize))
					tag.Tag = buffer;
			}

			ImGui::SameLine();

			// Add Button
			{
				if (UI::IconButton("  " ICON_MDI_PLUS, "Add  "))
					ImGui::OpenPopup("AddComponentPopup");

				if (ImGui::BeginPopup("AddComponentPopup"))
				{
					const float filterCursorPosX = ImGui::GetCursorPosX();
					m_Filter.Draw("###PropertiesFilter", ImGui::GetContentRegionAvail().x);

					if (!m_Filter.IsActive())
					{
						ImGui::SameLine();
						ImGui::SetCursorPosX(filterCursorPosX + ImGui::GetFontSize() * 0.5f);
						ImGui::TextUnformatted(ICON_MDI_MAGNIFY " Search...");
					}

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);

					DrawAddComponent<SpriteRendererComponent>(entity, ICON_MDI_IMAGE_SIZE_SELECT_ACTUAL " Sprite Renderer", "2D");
					DrawAddComponent<Rigidbody2DComponent>(entity, ICON_MDI_SOCCER " Rigidbody 2D", "2D");
					DrawAddComponent<BoxCollider2DComponent>(entity, ICON_MDI_CHECKBOX_BLANK_OUTLINE " Box Collider 2D", "2D");
					DrawAddComponent<CircleCollider2DComponent>(entity, ICON_MDI_CIRCLE_OUTLINE " Circle Collider 2D", "2D");
					DrawAddComponent<PolygonCollider2DComponent>(entity, ICON_MDI_CIRCLE_OUTLINE " Polygon Collider 2D", "2D");
					DrawAddComponent<DistanceJoint2DComponent>(entity, ICON_MDI_VECTOR_LINE " Distance Joint 2D", "2D");
					DrawAddComponent<SpringJoint2DComponent>(entity, ICON_MDI_VECTOR_LINE " Spring Joint 2D", "2D");
					DrawAddComponent<HingeJoint2DComponent>(entity, ICON_MDI_ANGLE_ACUTE " Hinge Joint 2D", "2D");
					DrawAddComponent<SliderJoint2DComponent>(entity, ICON_MDI_VIEW_AGENDA " Slider Joint 2D", "2D");
					DrawAddComponent<WheelJoint2DComponent>(entity, ICON_MDI_CAR " Wheel Joint 2D", "2D");
					DrawAddComponent<BuoyancyEffector2DComponent>(entity, ICON_MDI_WATER " Buoyancy Effector 2D", "2D");

					DrawAddComponent<SkyLightComponent>(entity, ICON_MDI_EARTH " Sky Light", "3D");
					DrawAddComponent<LightComponent>(entity, ICON_MDI_LIGHTBULB " Light", "3D");
					DrawAddComponent<MeshComponent>(entity, ICON_MDI_VECTOR_SQUARE " Mesh", "3D");
					DrawAddComponent<RigidbodyComponent>(entity, ICON_MDI_SOCCER " Rigidbody", "3D");
					DrawAddComponent<BoxColliderComponent>(entity, ICON_MDI_CHECKBOX_BLANK_OUTLINE " Box Collider", "3D");
					DrawAddComponent<SphereColliderComponent>(entity, ICON_MDI_CIRCLE_OUTLINE " Sphere Collider", "3D");
					DrawAddComponent<CapsuleColliderComponent>(entity, ICON_MDI_CIRCLE_OUTLINE " Capsule Collider", "3D");
					DrawAddComponent<TaperedCapsuleColliderComponent>(entity, ICON_MDI_CIRCLE_OUTLINE " Tapered Capsule Collider", "3D");
					DrawAddComponent<CylinderColliderComponent>(entity, ICON_MDI_CIRCLE_OUTLINE " Cylinder Collider", "3D");

					DrawAddComponent<AudioSourceComponent>(entity, ICON_MDI_VOLUME_MEDIUM " Audio", "Audio");
					DrawAddComponent<AudioListenerComponent>(entity, ICON_MDI_CIRCLE_SLICE_8 " Audio Listener", "Audio");

					DrawAddComponent<ParticleSystemComponent>(entity, ICON_MDI_LAMP " Particle System");
					DrawAddComponent<CameraComponent>(entity, ICON_MDI_CAMERA " Camera");
					DrawAddComponent<ScriptComponent>(entity, ICON_MDI_POUND_BOX " Script");

					ImGui::PopStyleVar();
					ImGui::EndPopup();
				}
			}

			ImGui::SameLine();

			// Lock Button
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + framePadding.x);
				const char* icon = m_Locked ? ICON_MDI_LOCK : ICON_MDI_LOCK_OPEN_OUTLINE;
				if (UI::ToggleButton(icon, m_Locked, lockButtonSize))
					m_Locked = !m_Locked;
			}

			ImGui::SetNextItemWidth(regionX - framePadding.x);

			const char* current;
			const auto& it = Scene::LayerCollisionMask.find(tag.Layer);
			if (it != Scene::LayerCollisionMask.end())
				current = it->second.Name.c_str();
			else
				current = Scene::LayerCollisionMask[Scene::DefaultLayer].Name.c_str();

			if (ImGui::BeginCombo("##LayerName", current))
			{
				for (const auto& [layer, layerData] : Scene::LayerCollisionMask)
				{
					const bool isSelected = current == layerData.Name;
					if (ImGui::Selectable(Scene::LayerCollisionMask.at(layer).Name.c_str(), isSelected))
					{
						current = Scene::LayerCollisionMask.at(layer).Name.c_str();
						tag.Layer = layer;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		ImGui::BeginChild("PropertiesBody");
		DrawComponent<TransformComponent>(ICON_MDI_VECTOR_LINE " Transform", entity, [](TransformComponent& component)
		{
			UI::BeginProperties();

			UI::DrawVec3Control("Translation", component.Translation);
			
			glm::vec3 rotation = glm::degrees(component.Rotation);
			UI::DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);

			UI::DrawVec3Control("Scale", component.Scale, nullptr, 1.0f);

			UI::EndProperties();
		}, false);

		DrawComponent<CameraComponent>(ICON_MDI_CAMERA " Camera", entity, [](CameraComponent& component)
		{
			auto& camera = component.Camera;

			UI::BeginProperties();

			UI::Property("Primary", component.Primary);

			auto projectionType = camera.GetProjectionType();
			if (UI::PropertyEnum("Projection", projectionType))
				camera.SetProjectionType(projectionType);

			if(camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if(UI::Property("Vertical FOV", verticalFov))
					camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));

				float perspectiveNear = camera.GetPerspectiveNearClip();
				if(UI::Property("Near Clip", perspectiveNear))
					camera.SetPerspectiveNearClip(perspectiveNear);
				
				float perspectiveFar = camera.GetPerspectiveFarClip();
				if(UI::Property("Far Clip", perspectiveFar))
					camera.SetPerspectiveFarClip(perspectiveFar);
			}

			if(camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if(UI::Property("Size", orthoSize))
					camera.SetOrthographicSize(orthoSize);

				float orthoNear = camera.GetOrthographicNearClip();
				if(UI::Property("Near Clip", orthoNear))
					camera.SetOrthographicNearClip(orthoNear);
				
				float orthoFar = camera.GetOrthographicFarClip();
				if(UI::Property("Far Clip", orthoFar))
					camera.SetOrthographicFarClip(orthoFar);

				UI::Property("Fixed Aspect Ratio", component.FixedAspectRatio);
			}

			UI::EndProperties();
		});
		
		DrawComponent<SpriteRendererComponent>(ICON_MDI_IMAGE_SIZE_SELECT_ACTUAL " Sprite Renderer", entity, [&entity](SpriteRendererComponent& component)
		{
			UI::BeginProperties();
			UI::PropertyVector("Color", component.Color, true);
			UI::Property("Texture", component.Texture);
			if (UI::Property("Sorting Order", component.SortingOrder))
				entity.GetScene()->SortForSprites();
			UI::PropertyVector("Tiling", component.Tiling);
			UI::PropertyVector("Offset", component.Offset);
			UI::EndProperties();
		});

		DrawComponent<MeshComponent>(ICON_MDI_VECTOR_SQUARE " Mesh", entity, [](MeshComponent& component)
		{
			if (ImGui::Button(component.MeshGeometry ? component.MeshGeometry->GetFilepath() : "null", { ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() }))
			{
				const std::string filepath = FileDialogs::OpenFile("Mesh (*.assbin)\0*.assbin\0(*.obj)\0*.obj\0(*.fbx)\0*.fbx\0");
				if (!filepath.empty())
				{
					component.MeshGeometry = CreateRef<Mesh>(filepath.c_str());
					return;
				}
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					component.MeshGeometry = CreateRef<Mesh>(static_cast<char*>(payload->Data));
					ImGui::EndDragDropTarget();
					return;
				}
				ImGui::EndDragDropTarget();
			}

			if (component.MeshGeometry)
			{
				UI::BeginProperties();

				const size_t submeshCount = component.MeshGeometry->GetSubmeshCount();
				if (submeshCount > 1)
					UI::Property<size_t>("Submesh Index", component.SubmeshIndex, 0, submeshCount - 1);

				UI::EndProperties();

				constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
					| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

				if (ImGui::TreeNodeEx("Material Properties", treeNodeFlags))
				{
					if (component.MeshGeometry->GetSubmeshCount() > 0)
					{
						DrawMaterialProperties(component.MeshGeometry->GetSubmesh(component.SubmeshIndex).Mat);
					}
					ImGui::TreePop();
				}
			}
		});

		DrawComponent<SkyLightComponent>(ICON_MDI_EARTH " Sky Light", entity, [](SkyLightComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Texture", component.Texture, component.Texture == nullptr ? 0 : component.Texture->GetHRDRendererID());
			UI::Property("Intensity", component.Intensity);
			UI::Property("Rotation", component.Rotation, 0.0f, 360.0f);
			UI::EndProperties();
		});

		DrawComponent<LightComponent>(ICON_MDI_LIGHTBULB " Light", entity, [](LightComponent& component)
		{
			UI::BeginProperties();
			UI::PropertyEnum("Light Type", component.Type);

			if (UI::Property("Use color temperature mode", component.UseColorTemperatureMode) && component.UseColorTemperatureMode)
			{
				ColorUtils::TempratureToColor(component.Temperature, component.Color);
			}

			if (component.UseColorTemperatureMode)
			{
				if (UI::Property<uint32_t>("Temperature (K)", component.Temperature, 1000, 40000))
					ColorUtils::TempratureToColor(component.Temperature, component.Color);
			}
			else
			{
				UI::PropertyVector("Color", component.Color, true);
			}

			if (UI::Property("Intensity", component.Intensity) && component.Intensity < 0.0f)
			{
				component.Intensity = 0.0f;
			}

			ImGui::Spacing();

			if (component.Type == LightComponent::LightType::Point)
			{
				UI::Property("Range", component.Range);
			}
			else if (component.Type == LightComponent::LightType::Spot)
			{
				UI::Property("Range", component.Range);
				float degrees = glm::degrees(component.OuterCutOffAngle);
				if (UI::Property("Outer Cut-Off Angle", degrees, 1.0f, 90.0f))
					component.OuterCutOffAngle = glm::radians(degrees);
				degrees = glm::degrees(component.CutOffAngle);
				if (UI::Property("Cut-Off Angle", degrees, 1.0f, 90.0f))
					component.CutOffAngle = glm::radians(degrees);

				if (component.Range < 0.1f)
					component.Range = 0.1f;
				if (component.OuterCutOffAngle < component.CutOffAngle)
					component.CutOffAngle = component.OuterCutOffAngle;
				if (component.CutOffAngle > component.OuterCutOffAngle)
					component.OuterCutOffAngle = component.CutOffAngle;
			}
			else
			{
				UI::PropertyEnum("Shadow Quality Type", component.ShadowQuality);

				uint64_t textureID = component.ShadowMapFramebuffer->GetDepthAttachmentRendererID();
				if (textureID == 0)
					textureID = AssetManager::BlackTexture()->GetRendererID();
				ImGui::Image(reinterpret_cast<ImTextureID>(textureID), ImVec2{ 256, 256 }, ARC_UI_UV_0, ARC_UI_UV_1);
			}

			UI::EndProperties();
		});

		DrawComponent<ParticleSystemComponent>(ICON_MDI_LAMP " Particle System", entity, [](const ParticleSystemComponent& component)
		{
			auto& props = component.System->GetProperties();

			ImGui::Text("Active Particles Count: %u", component.System->GetActiveParticleCount());
			ImGui::BeginDisabled(props.Looping);
			if (ImGui::Button(ICON_MDI_PLAY))
				component.System->Play();
			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_STOP))
				component.System->Stop();
			ImGui::EndDisabled();

			ImGui::Separator();

			UI::BeginProperties();
			UI::Property("Duration", props.Duration);
			if (UI::Property("Looping", props.Looping))
			{
				if (props.Looping)
					component.System->Play();
			}
			UI::Property("Start Delay", props.StartDelay);
			UI::Property("Start Lifetime", props.StartLifetime);
			UI::PropertyVector("Start Velocity", props.StartVelocity);
			UI::PropertyVector("Start Color", props.StartColor, true);
			UI::PropertyVector("Start Size", props.StartSize);
			UI::PropertyVector("Start Rotation", props.StartRotation);
			UI::Property("Gravity Modifier", props.GravityModifier);
			UI::Property("Simulation Speed", props.SimulationSpeed);
			UI::Property("Play On Awake", props.PlayOnAwake);
			UI::Property("Max Particles", props.MaxParticles);
			UI::EndProperties();

			ImGui::Separator();

			UI::BeginProperties();
			UI::Property("Rate Over Time", props.RateOverTime);
			UI::Property("Rate Over Distance", props.RateOverDistance);
			UI::Property("Burst Count", props.BurstCount);
			UI::Property("Burst Time", props.BurstTime);
			UI::PropertyVector("Position Start", props.PositionStart);
			UI::PropertyVector("Position End", props.PositionEnd);
			UI::Property("Texture", props.Texture);
			UI::EndProperties();

			DrawParticleOverLifetimeModule("Velocity Over Lifetime", props.VelocityOverLifetime);
			DrawParticleOverLifetimeModule("Force Over Lifetime", props.ForceOverLifetime);
			DrawParticleOverLifetimeModule("Color Over Lifetime", props.ColorOverLifetime, true);
			DrawParticleBySpeedModule("Color By Speed", props.ColorBySpeed, true);
			DrawParticleOverLifetimeModule("Size Over Lifetime", props.SizeOverLifetime);
			DrawParticleBySpeedModule("Size By Speed", props.SizeBySpeed);
			DrawParticleOverLifetimeModule("Rotation Over Lifetime", props.RotationOverLifetime, false, true);
			DrawParticleBySpeedModule("Rotation By Speed", props.RotationBySpeed, false, true);
		});

		DrawComponent<Rigidbody2DComponent>(ICON_MDI_SOCCER " Rigidbody 2D", entity, [](Rigidbody2DComponent& component)
		{
			UI::BeginProperties();

			UI::PropertyEnum("Body Type", component.Type);

			if (component.Type == Rigidbody2DComponent::BodyType::Dynamic)
			{
				bool tmpBool = component.AutoMass;
				if (UI::Property("Auto Mass", tmpBool))
					component.AutoMass = tmpBool;

				if (!component.AutoMass)
					UI::Property("Mass", component.Mass, 0.01f, 10000.0f);
				UI::Property("Linear Drag", component.LinearDrag);
				UI::Property("Angular Drag", component.AngularDrag);
				UI::Property("Gravity Scale", component.GravityScale);

				tmpBool = component.AllowSleep;
				if (UI::Property("Allow Sleep", tmpBool))
					component.AllowSleep = tmpBool;

				tmpBool = component.Awake;
				if (UI::Property("Awake", tmpBool))
					component.Awake = tmpBool;

				tmpBool = component.Continuous;
				if (UI::Property("Continuous", tmpBool))
					component.Continuous = tmpBool;

				tmpBool = component.Interpolation;
				if (UI::Property("Interpolation", tmpBool))
					component.Interpolation = tmpBool;

				tmpBool = component.FreezeRotation;
				if (UI::Property("Freeze Rotation", tmpBool))
					component.FreezeRotation = tmpBool;

				component.LinearDrag = glm::max(component.LinearDrag, 0.0f);
				component.AngularDrag = glm::max(component.AngularDrag, 0.0f);
			}

			UI::EndProperties();
		});

		DrawComponent<BoxCollider2DComponent>(ICON_MDI_CHECKBOX_BLANK_OUTLINE " Box Collider 2D", entity, [](BoxCollider2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Is Sensor", component.IsSensor);
			UI::PropertyVector("Size", component.Size);
			UI::PropertyVector("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::EndProperties();
			
			if (!component.IsSensor)
			{
				ImGui::Spacing();
				UI::BeginProperties();
				UI::Property("Friction", component.Friction);
				UI::Property("Restitution", component.Restitution);
				UI::EndProperties();
			}
		});

		DrawComponent<CircleCollider2DComponent>(ICON_MDI_CIRCLE_OUTLINE " Circle Collider 2D", entity, [](CircleCollider2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Is Sensor", component.IsSensor);
			UI::Property("Radius", component.Radius);
			UI::PropertyVector("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::EndProperties();
			
			if (!component.IsSensor)
			{
				ImGui::Spacing();
				UI::BeginProperties();
				UI::Property("Friction", component.Friction);
				UI::Property("Restitution", component.Restitution);
				UI::EndProperties();
			}
		});

		DrawComponent<PolygonCollider2DComponent>(ICON_MDI_CIRCLE_OUTLINE " Polygon Collider 2D", entity, [](PolygonCollider2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Is Sensor", component.IsSensor);
			UI::PropertyVector("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::EndProperties();
			
			UI::Property("Points", component.Points, glm::vec3(0.0f), 3);

			if (!component.IsSensor)
			{
				ImGui::Spacing();
				UI::BeginProperties();
				UI::Property("Friction", component.Friction);
				UI::Property("Restitution", component.Restitution);
				UI::EndProperties();
			}
		});

		DrawComponent<DistanceJoint2DComponent>(ICON_MDI_VECTOR_LINE " Distance Joint 2D", entity, [&entity](DistanceJoint2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Enable Collision", component.EnableCollision);
			UI::PropertyComponent<Rigidbody2DComponent>("Connected Rigidbody 2D", "Rigidbody 2D", entity.GetScene(), component.ConnectedRigidbody, nullptr);
			UI::PropertyVector("Anchor", component.Anchor);
			UI::PropertyVector("Connected Anchor", component.ConnectedAnchor);
			UI::Property("Auto Distance", component.AutoDistance);
			if (!component.AutoDistance)
				UI::Property("Distance", component.Distance);
			UI::Property("Min Distance", component.MinDistance);
			UI::Property("Max Distance By", component.MaxDistanceBy);
			UI::Property("Break Force", component.BreakForce);
			UI::EndProperties();

			component.MinDistance = glm::max(component.MinDistance, 0.0f);
			component.MaxDistanceBy = glm::max(component.MaxDistanceBy, 0.0f);
		});

		DrawComponent<SpringJoint2DComponent>(ICON_MDI_VECTOR_LINE " Spring Joint 2D", entity, [&entity](SpringJoint2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Enable Collision", component.EnableCollision);
			UI::PropertyComponent<Rigidbody2DComponent>("Connected Rigidbody 2D", "Rigidbody 2D", entity.GetScene(), component.ConnectedRigidbody, nullptr);
			UI::PropertyVector("Anchor", component.Anchor);
			UI::PropertyVector("Connected Anchor", component.ConnectedAnchor);
			UI::Property("Auto Distance", component.AutoDistance);
			if (!component.AutoDistance)
				UI::Property("Distance", component.Distance);
			UI::Property("Min Distance", component.MinDistance);
			UI::Property("Max Distance By", component.MaxDistanceBy);
			UI::Property("Frequency", component.Frequency);
			UI::Property("Damping Ratio", component.DampingRatio, 0.0f, 1.0f);
			UI::Property("Break Force", component.BreakForce);
			UI::EndProperties();

			component.MinDistance = glm::max(component.MinDistance, 0.0f);
			component.MaxDistanceBy = glm::max(component.MaxDistanceBy, 0.0f);
			component.Frequency = glm::max(component.Frequency, 0.0f);
		});

		DrawComponent<HingeJoint2DComponent>(ICON_MDI_ANGLE_ACUTE " Hinge Joint 2D", entity, [&entity](HingeJoint2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Enable Collision", component.EnableCollision);
			UI::PropertyComponent<Rigidbody2DComponent>("Connected Rigidbody 2D", "Rigidbody 2D", entity.GetScene(), component.ConnectedRigidbody, nullptr);
			UI::PropertyVector("Anchor", component.Anchor);

			UI::Property("Use Limits", component.UseLimits);
			if (component.UseLimits)
			{
				ImGui::Indent();
				float degrees = glm::degrees(component.LowerAngle);
				if (UI::Property("Lower Angle", degrees, 0.0f, 359.0f))
					component.LowerAngle = glm::radians(degrees);
				component.UpperAngle = glm::max(component.LowerAngle, component.UpperAngle);
				degrees = glm::degrees(component.UpperAngle);
				if (UI::Property("Upper Angle", degrees, glm::degrees(component.LowerAngle), 359.9f))
					component.UpperAngle = glm::radians(degrees);
				ImGui::Unindent();
			}

			UI::Property("Use Motor", component.UseMotor);
			if (component.UseMotor)
			{
				ImGui::Indent();
				UI::Property("Motor Speed", component.MotorSpeed);
				UI::Property("Max Motor Torque", component.MaxMotorTorque);
				ImGui::Unindent();
			}

			UI::Property("Break Force", component.BreakForce);
			UI::Property("Break Torque", component.BreakTorque);
			UI::EndProperties();
		});

		DrawComponent<SliderJoint2DComponent>(ICON_MDI_VIEW_AGENDA " Slider Joint 2D", entity, [&entity](SliderJoint2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Enable Collision", component.EnableCollision);
			UI::PropertyComponent<Rigidbody2DComponent>("Connected Rigidbody 2D", "Rigidbody 2D", entity.GetScene(), component.ConnectedRigidbody, nullptr);
			UI::PropertyVector("Anchor", component.Anchor);

			float degrees = glm::degrees(component.Angle);
			if (UI::Property("Angle", degrees))
				component.Angle = glm::radians(degrees);
			UI::Property("Use Limits", component.UseLimits);
			if (component.UseLimits)
			{
				ImGui::Indent();
				UI::Property("Lower Translation", component.LowerTranslation);
				component.UpperTranslation = glm::max(component.LowerTranslation, component.UpperTranslation);
				UI::Property("Upper Angle", component.UpperTranslation);
				ImGui::Unindent();
			}

			UI::Property("Use Motor", component.UseMotor);
			if (component.UseMotor)
			{
				ImGui::Indent();
				UI::Property("Motor Speed", component.MotorSpeed);
				UI::Property("Max Motor Force", component.MaxMotorForce);
				ImGui::Unindent();
			}

			UI::Property("Break Force", component.BreakForce);
			UI::Property("Break Torque", component.BreakTorque);
			UI::EndProperties();
		});

		DrawComponent<WheelJoint2DComponent>(ICON_MDI_CAR " Wheel Joint 2D", entity, [&entity](WheelJoint2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Enable Collision", component.EnableCollision);
			UI::PropertyComponent<Rigidbody2DComponent>("Connected Rigidbody 2D", "Rigidbody 2D", entity.GetScene(), component.ConnectedRigidbody, nullptr);
			UI::PropertyVector("Anchor", component.Anchor);

			UI::Property("Frequency", component.Frequency);
			UI::Property("Damping Ratio", component.DampingRatio);
			UI::Property("Use Limits", component.UseLimits);
			if (component.UseLimits)
			{
				ImGui::Indent();
				UI::Property("Lower Translation", component.LowerTranslation);
				component.UpperTranslation = glm::max(component.LowerTranslation, component.UpperTranslation);
				UI::Property("Upper Angle", component.UpperTranslation);
				ImGui::Unindent();
			}

			UI::Property("Use Motor", component.UseMotor);
			if (component.UseMotor)
			{
				ImGui::Indent();
				UI::Property("Motor Speed", component.MotorSpeed);
				UI::Property("Max Motor Torque", component.MaxMotorTorque);
				ImGui::Unindent();
			}

			UI::Property("Break Force", component.BreakForce);
			UI::Property("Break Torque", component.BreakTorque);
			UI::EndProperties();
		});

		DrawComponent<BuoyancyEffector2DComponent>(ICON_MDI_WATER " Buoyancy Effector 2D", entity, [](BuoyancyEffector2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Density", component.Density);
			UI::Property("Drag Multiplier", component.DragMultiplier);
			UI::Property("Flip Gravity", component.FlipGravity);
			UI::Property("Flow Magnitude", component.FlowMagnitude);
			float degrees = glm::degrees(component.FlowAngle);
			if (UI::Property("Flow Angle", degrees))
				component.FlowAngle = glm::radians(degrees);
			UI::EndProperties();
		});

		DrawComponent<RigidbodyComponent>(ICON_MDI_SOCCER " Rigidbody", entity, [](RigidbodyComponent& component)
		{
			UI::BeginProperties();

			UI::PropertyEnum("Body Type", component.Type);

			if (component.Type == RigidbodyComponent::BodyType::Dynamic)
			{
				bool tmpBool = component.AutoMass;
				if (UI::Property("Auto Mass", tmpBool))
					component.AutoMass = tmpBool;

				if (!component.AutoMass)
					UI::Property("Mass", component.Mass, 0.01f, 10000.0f);
				UI::Property("Linear Drag", component.LinearDrag);
				UI::Property("Angular Drag", component.AngularDrag);
				UI::Property("Gravity Scale", component.GravityScale);

				tmpBool = component.AllowSleep;
				if (UI::Property("Allow Sleep", tmpBool))
					component.AllowSleep = tmpBool;

				tmpBool = component.Awake;
				if (UI::Property("Awake", tmpBool))
					component.Awake = tmpBool;

				tmpBool = component.Continuous;
				if (UI::Property("Continuous", tmpBool))
					component.Continuous = tmpBool;

				tmpBool = component.Interpolation;
				if (UI::Property("Interpolation", tmpBool))
					component.Interpolation = tmpBool;

				component.LinearDrag = glm::max(component.LinearDrag, 0.0f);
				component.AngularDrag = glm::max(component.AngularDrag, 0.0f);
			}

			bool sensor = component.IsSensor;
			if (UI::Property("Is Sensor", sensor))
				component.IsSensor = sensor;

			UI::EndProperties();
		});

		DrawComponent<BoxColliderComponent>(ICON_MDI_CHECKBOX_BLANK_OUTLINE " Box Collider", entity, [](BoxColliderComponent& component)
		{
			UI::BeginProperties();
			UI::PropertyVector("Size", component.Size);
			UI::PropertyVector("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction, 0.0f, 1.0f);
			UI::Property("Restitution", component.Restitution, 0.0f, 1.0f);
			UI::EndProperties();

			component.Density = glm::max(component.Density, 0.001f);
		});

		DrawComponent<SphereColliderComponent>(ICON_MDI_CIRCLE_OUTLINE " Sphere Collider", entity, [](SphereColliderComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Radius", component.Radius);
			UI::PropertyVector("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction, 0.0f, 1.0f);
			UI::Property("Restitution", component.Restitution, 0.0f, 1.0f);
			UI::EndProperties();

			component.Density = glm::max(component.Density, 0.001f);
		});

		DrawComponent<CapsuleColliderComponent>(ICON_MDI_CIRCLE_OUTLINE " Capsule Collider", entity, [](CapsuleColliderComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Height", component.Height);
			UI::Property("Radius", component.Radius);
			UI::PropertyVector("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction, 0.0f, 1.0f);
			UI::Property("Restitution", component.Restitution, 0.0f, 1.0f);
			UI::EndProperties();

			component.Density = glm::max(component.Density, 0.001f);
		});

		DrawComponent<TaperedCapsuleColliderComponent>(ICON_MDI_CIRCLE_OUTLINE " Tapered Capsule Collider", entity, [](TaperedCapsuleColliderComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Height", component.Height);
			UI::Property("Top Radius", component.TopRadius);
			UI::Property("Bottom Radius", component.BottomRadius);
			UI::PropertyVector("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction, 0.0f, 1.0f);
			UI::Property("Restitution", component.Restitution, 0.0f, 1.0f);
			UI::EndProperties();

			component.Density = glm::max(component.Density, 0.001f);
		});

		DrawComponent<CylinderColliderComponent>(ICON_MDI_CIRCLE_OUTLINE " Cylinder Collider", entity, [](CylinderColliderComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Height", component.Height);
			UI::Property("Radius", component.Radius);
			UI::PropertyVector("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction, 0.0f, 1.0f);
			UI::Property("Restitution", component.Restitution, 0.0f, 1.0f);
			UI::EndProperties();

			component.Density = glm::max(component.Density, 0.001f);
		});

		DrawComponent<ScriptComponent>(ICON_MDI_POUND_BOX " Script", entity, [this, &entity, &framePadding](ScriptComponent& component)
		{
			ImGui::Spacing();

			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - (UI::GetIconButtonSize("  " ICON_MDI_PLUS, "Add  ").x + framePadding.x * 2.0f));
			if (UI::IconButton("  " ICON_MDI_PLUS, "Add  "))
				ImGui::OpenPopup("ScriptAddPopup");

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			ImGui::Separator();
			ImGui::Spacing();

			const auto& classes = ScriptEngine::GetClasses();
			if (ImGui::BeginPopup("ScriptAddPopup"))
			{
				if (!classes.empty())
				{
					const float filterCursorPosX = ImGui::GetCursorPosX();
					m_Filter.Draw("###PropertiesFilter", ImGui::GetContentRegionAvail().x);

					if (!m_Filter.IsActive())
					{
						ImGui::SameLine();
						ImGui::SetCursorPosX(filterCursorPosX + ImGui::GetFontSize() * 0.5f);
						ImGui::TextUnformatted(ICON_MDI_MAGNIFY " Search...");
					}
				}

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::PopupItemSpacing);
				for (const auto& [name, _] : classes)
				{
					const bool notFound = std::ranges::find(component.Classes, name) == component.Classes.end();
					if (notFound && (!m_Filter.IsActive() || (m_Filter.IsActive() && m_Filter.PassFilter(name.c_str()))))
					{
						if (ImGui::MenuItem(name.c_str()))
						{
							component.Classes.emplace_back(name);
						}
					}
				}
				ImGui::PopStyleVar();
				ImGui::EndPopup();
			}
			
			DrawFields(entity, component);
		});

		DrawComponent<AudioSourceComponent>(ICON_MDI_VOLUME_MEDIUM " Audio", entity, [&entity](AudioSourceComponent& component)
		{
			auto& config = component.Config;

			const char* filepath = component.Source ? component.Source->GetPath() : "Drop an audio file";
			const float x = ImGui::GetContentRegionAvail().x;
			const float y = ImGui::GetFrameHeight();
			ImGui::Button(filepath, { x, y });
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const char* path = static_cast<char*>(payload->Data);
					const auto ext = StringUtils::GetExtension(path);
					if (ext == "mp3" || ext == "wav")
						component.Source = CreateRef<AudioSource>(path);
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::Spacing();
			
			UI::BeginProperties();
			UI::Property("Volume Multiplier", config.VolumeMultiplier);
			UI::Property("Pitch Multiplier", config.PitchMultiplier);
			UI::Property("Play On Awake", config.PlayOnAwake);
			UI::Property("Looping", config.Looping);
			UI::EndProperties();

			ImGui::Spacing();
			if (ImGui::Button(ICON_MDI_PLAY "Play ") && component.Source)
				component.Source->Play();
			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_PAUSE "Pause ") && component.Source)
				component.Source->Pause();
			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_STOP "Stop ") && component.Source)
				component.Source->Stop();
			ImGui::Spacing();

			UI::BeginProperties();
			UI::Property("Spatialization", config.Spatialization);

			if (config.Spatialization)
			{
				ImGui::Indent();
				UI::PropertyEnum("Attenuation Model", config.AttenuationModel);
				UI::Property("Roll Off", config.RollOff);
				UI::Property("Min Gain", config.MinGain);
				UI::Property("Max Gain", config.MaxGain);
				UI::Property("Min Distance", config.MinDistance);
				UI::Property("Max Distance", config.MaxDistance);
				float degrees = glm::degrees(config.ConeInnerAngle);
				if (UI::Property("Cone Inner Angle", degrees))
					config.ConeInnerAngle = glm::radians(degrees);
				degrees = glm::degrees(config.ConeOuterAngle);
				if (UI::Property("Cone Outer Angle", degrees))
					config.ConeOuterAngle = glm::radians(degrees);
				UI::Property("Cone Outer Gain", config.ConeOuterGain);
				UI::Property("Doppler Factor", config.DopplerFactor);
				ImGui::Unindent();
			}
			UI::EndProperties();

			if (component.Source)
			{
				const glm::mat4 inverted = glm::inverse(entity.GetWorldTransform());
				const glm::vec3 forward = normalize(glm::vec3(inverted[2]));
				component.Source->SetConfig(config);
				component.Source->SetPosition(entity.GetTransform().Translation);
				component.Source->SetDirection(-forward);
			}
		});

		DrawComponent<AudioListenerComponent>(ICON_MDI_CIRCLE_SLICE_8 " Audio Listener", entity, [](AudioListenerComponent& component)
		{
			auto& config = component.Config;
			UI::BeginProperties();
			UI::Property("Active", component.Active);
			float degrees = glm::degrees(config.ConeInnerAngle);
			if (UI::Property("Cone Inner Angle", degrees))
				config.ConeInnerAngle = glm::radians(degrees);
			degrees = glm::degrees(config.ConeOuterAngle);
			if (UI::Property("Cone Outer Angle", degrees))
				config.ConeOuterAngle = glm::radians(degrees);
			UI::Property("Cone Outer Gain", config.ConeOuterGain);
			UI::EndProperties();
		});

		ImGui::EndChild();
	}

	void PropertiesPanel::DrawFileProperties(const char* filepath)
	{
		const auto name = StringUtils::GetNameWithExtension(filepath);
		const auto ext = StringUtils::GetExtension(name);

		if (ext == "prefab")
		{
			static Entity prefab;
			if (m_Scene && prefab)
			{
				DrawComponents(prefab);
			}
			else
			{
				m_Scene = CreateScope<Scene>();
				prefab = EntitySerializer::DeserializeEntityAsPrefab(filepath, *m_Scene);
			}
		}
		else
		{
			if (m_Scene)
				m_Scene.reset();

			ImGui::TextUnformatted(name.data());
		}
	}

	template<typename Component>
	void PropertiesPanel::DrawAddComponent(Entity entity, const char* name, const char* category) const
	{
		if (!entity.HasComponent<Component>())
		{
			if (!m_Filter.IsActive())
			{
				if (category)
				{
					if (ImGui::BeginMenu(category))
					{
						if (ImGui::MenuItem(name))
						{
							entity.AddComponent<Component>();
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndMenu();
					}
				}
				else
				{
					if (ImGui::MenuItem(name))
					{
						entity.AddComponent<Component>();
						ImGui::CloseCurrentPopup();
					}
				}
			}
			else
			{
				if (m_Filter.IsActive() && m_Filter.PassFilter(name))
				{
					if (ImGui::MenuItem(name))
					{
						entity.AddComponent<Component>();
						ImGui::CloseCurrentPopup();
					}
				}
			}
		}
	}
}
