#include "PropertiesPanel.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Arc/Utils/PlatformUtils.h"
#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"
#include "../EditorLayer.h"

namespace ArcEngine
{
	void PropertiesPanel::OnImGuiRender()
	{
		ARC_PROFILE_SCOPE();

		if (OnBegin())
		{
			if (m_Context && m_Context.GetScene())
				DrawComponents(m_Context);

			OnEnd();
		}
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const eastl::string& name, Entity entity, UIFunction uiFunction, const bool removable = true)
	{
		ARC_PROFILE_SCOPE();

		if(entity.HasComponent<T>())
		{
			static const ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_FramePadding;

			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + lineHeight * 0.25f);

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeFlags, name.c_str());

			bool removeComponent = false;
			if(removable)
			{
				ImGui::SameLine(ImGui::GetContentRegionMax().x - lineHeight);
				if(ImGui::Button(ICON_MDI_SETTINGS, ImVec2{ lineHeight, lineHeight }))
					ImGui::OpenPopup("ComponentSettings");

				if(ImGui::BeginPopup("ComponentSettings"))
				{
					if(ImGui::MenuItem("Remove Component"))
						removeComponent = true;
					
					ImGui::EndPopup();
				}
			}
			ImGui::PopStyleVar();

			if(open)
			{
				ARC_PROFILE_SCOPE("UI Function");

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().IndentSpacing / 2);
				uiFunction(component);
				ImGui::TreePop();
			}

			if(removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	static void DrawFields(ScriptComponent& component)
	{
		UI::BeginProperties();

		// Public Fields
		eastl::hash_map<eastl::string, Field>* fieldMap = ScriptEngine::GetFields(component.ClassName.c_str());
		if (fieldMap)
		{
			for (auto& [name, field] : *fieldMap)
			{
				switch (field.Type)
				{
					case Field::FieldType::Bool:
					{
						bool value = field.GetValue<bool>(component.Handle);
						if (UI::Property(field.Name.c_str(), value))
							field.SetValue(component.Handle, value);
						break;
					}
					case Field::FieldType::Float:
					{
						float value = field.GetValue<float>(component.Handle);
						if (UI::Property(field.Name.c_str(), value))
							field.SetValue(component.Handle, value);
						break;
					}
					case Field::FieldType::Int:
					{
						int32_t value = field.GetValue<int32_t>(component.Handle);
						if (UI::Property(field.Name.c_str(), value))
							field.SetValue(component.Handle, value);
						break;
					}
					case Field::FieldType::UnsignedInt:
					{
						uint32_t value = field.GetValue<uint32_t>(component.Handle);
						if (UI::Property(field.Name.c_str(), value))
							field.SetValue(component.Handle, value);
						break;
					}
					case Field::FieldType::String:
					{
						eastl::string& value = field.GetValueString(component.Handle);
						if (UI::Property(field.Name.c_str(), value))
							field.SetValueString(component.Handle, value);
						break;
					}
					case Field::FieldType::Vec2:
					{
						glm::vec2 value = field.GetValue<glm::vec2>(component.Handle);
						if (UI::Property(field.Name.c_str(), value))
							field.SetValue(component.Handle, value);
						break;
					}
					case Field::FieldType::Vec3:
					{
						glm::vec3 value = field.GetValue<glm::vec3>(component.Handle);
						if (UI::Property(field.Name.c_str(), value))
							field.SetValue(component.Handle, value);
						break;
					}
					case Field::FieldType::Vec4:
					{
						glm::vec4 value = field.GetValue<glm::vec4>(component.Handle);
						if (UI::Property(field.Name.c_str(), value))
							field.SetValue(component.Handle, value);
						break;
					}
				}
			}
		}

		UI::EndProperties();
	}

	static void DrawMaterialProperties(Ref<Material>& material)
	{
		UI::BeginProperties();

		auto& materialProperties = material->GetShader()->GetMaterialProperties();
		for (auto& [n, property] : materialProperties)
		{
			const char* name = n.c_str();
			const char* displayName = n.c_str() + 11;

			switch (property.Type)
			{
				case MaterialPropertyType::None: break;
				case MaterialPropertyType::Sampler2D:
				{
					uint32_t slot = material->GetData<uint32_t>(name);
					uint32_t tex = material->GetTexture(slot) ? material->GetTexture(slot)->GetRendererID() : 0;
					Ref<Texture2D> tmp = material->GetTexture(slot);
					if (UI::Property(displayName, tmp, tex))
					{
						material->SetTexture(slot, tmp);
					}
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
					int32_t v = material->GetData<int32_t>(name);
					if (UI::Property(displayName, v))
						material->SetData(name, v);
					break;
				}
				case MaterialPropertyType::Float:
				{
					bool isSlider01 = n.find("01") != eastl::string::npos;
					float v = material->GetData<float>(name);
					if (isSlider01)
					{
						eastl::string displayName2 = displayName;
						displayName2[displayName2.size() - 2] = '\0';
						if (UI::Property(displayName2.c_str(), v, 0.0f, 1.0f))
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
					glm::vec2 v = material->GetData<glm::vec2>(name);
					if (UI::Property(displayName, v))
						material->SetData(name, v);
					break;
				}
				case MaterialPropertyType::Float3:
				{
					bool isColor = n.find("color") != eastl::string::npos || n.find("Color") != eastl::string::npos;
					glm::vec3 v = material->GetData<glm::vec3>(name);
					if (isColor)
					{
						if (UI::PropertyColor3(displayName, v))
							material->SetData(name, v);
					}
					else
					{
						if (UI::Property(displayName, v))
							material->SetData(name, v);
					}
					break;
				}
				case MaterialPropertyType::Float4:
				{
					bool isColor = n.find("color") != eastl::string::npos || n.find("Color") != eastl::string::npos;
					glm::vec4 v = material->GetData<glm::vec4>(name);
					if (isColor)
					{
						if (UI::PropertyColor4(displayName, v))
							material->SetData(name, v);
					}
					else
					{
						if (UI::Property(displayName, v))
							material->SetData(name, v);
					}
					break;
				}
			}
		}

		UI::EndProperties();
	}

	static void WriteMesh(const eastl::string& filepath)
	{
		ARC_PROFILE_SCOPE();

		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 80.0f);
		const uint32_t meshImportFlags =
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_PreTransformVertices |
			aiProcess_SortByPType |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords |
	        aiProcess_OptimizeMeshes |
			aiProcess_JoinIdenticalVertices |
			aiProcess_GlobalScale |
			aiProcess_ImproveCacheLocality |
	        aiProcess_ValidateDataStructure;

		const aiScene *scene = importer.ReadFile(filepath.c_str(), meshImportFlags);
		if (!scene)
		{
			ARC_CORE_ERROR("Could not import the file: {0}. Error: {1}", filepath, importer.GetErrorString());
			return;
		}

		Assimp::Exporter exporter;
		const aiExportFormatDesc* format = exporter.GetExportFormatDescription(0);
		auto lastDot = filepath.find_last_of(".");
		eastl::string path = filepath.substr(0, lastDot) + ".assbin";
		aiReturn ret = exporter.Export(scene, format->id, path.c_str(), meshImportFlags);
		if (ret != aiReturn_SUCCESS)
		{
			ARC_CORE_ERROR("Could not import the file: {0}. Error: {1}", filepath, exporter.GetErrorString());
			return;
		}
	}

	void PropertiesPanel::DrawComponents(Entity entity)
	{
		ARC_PROFILE_SCOPE();

		if(entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, tag.c_str());
			if(ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				tag = eastl::string(buffer);
		}

		ImGui::SameLine();
		{
			float frameHeight = ImGui::GetFrameHeight();
			ImVec2 region = ImGui::GetContentRegionMax();
			ImVec2 lockButtonSize = ImVec2(frameHeight * 1.5f, frameHeight);
			ImGui::SetCursorPosX(region.x - lockButtonSize.x);

			bool highlight = m_Locked;
			if (highlight)
				ImGui::PushStyleColor(ImGuiCol_Button, EditorTheme::HeaderSelectedColor);

			if (ImGui::Button(m_Locked ? ICON_MDI_LOCK : ICON_MDI_LOCK_OPEN_OUTLINE, lockButtonSize))
				m_Locked = !m_Locked;

			if (highlight)
				ImGui::PopStyleColor();
		}

		DrawComponent<TransformComponent>(ICON_MDI_VECTOR_LINE " Transform", entity, [](TransformComponent& component)
		{
			UI::BeginProperties();

			UI::DrawVec3Control("Translation", component.Translation);
			
			glm::vec3 rotation = glm::degrees(component.Rotation);
			UI::DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);

			UI::DrawVec3Control("Scale", component.Scale, 1.0f);

			UI::EndProperties();
		}, false);

		DrawComponent<CameraComponent>(ICON_MDI_CAMERA " Camera", entity, [](CameraComponent& component)
		{
			auto& camera = component.Camera;

			UI::BeginProperties();

			UI::Property("Primary", component.Primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			int projectionType = (int) camera.GetProjectionType();
			if (UI::Property("Projection", projectionType, projectionTypeStrings, 2))
				camera.SetProjectionType((SceneCamera::ProjectionType) projectionType);

			if(camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if(UI::Property("Vertical FOV", verticalFov, 0.1f))
					camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));

				float perspectiveNear = camera.GetPerspectiveNearClip();
				if(UI::Property("Near Clip", perspectiveNear, 0.1f))
					camera.SetPerspectiveNearClip(perspectiveNear);
				
				float perspectiveFar = camera.GetPerspectiveFarClip();
				if(UI::Property("Far Clip", perspectiveFar, 0.1f))
					camera.SetPerspectiveFarClip(perspectiveFar);
			}

			if(camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if(UI::Property("Size", orthoSize, 0.1f))
					camera.SetOrthographicSize(orthoSize);

				float orthoNear = camera.GetOrthographicNearClip();
				if(UI::Property("Near Clip", orthoNear, 0.1f))
					camera.SetOrthographicNearClip(orthoNear);
				
				float orthoFar = camera.GetOrthographicFarClip();
				if(UI::Property("Far Clip", orthoFar, 0.1f))
					camera.SetOrthographicFarClip(orthoFar);

				UI::Property("Fixed Aspect Ratio", component.FixedAspectRatio);
			}

			UI::EndProperties();
		});
		
		DrawComponent<SpriteRendererComponent>(ICON_MDI_IMAGE_SIZE_SELECT_ACTUAL " Sprite Renderer", entity, [](SpriteRendererComponent& component)
		{
			UI::BeginProperties();
			UI::PropertyColor4("Color", component.Color);
			UI::Property("Tiling Factor", component.TilingFactor, 0.1f);
			UI::Property("Texture", component.Texture);
			UI::EndProperties();
		});

		DrawComponent<MeshComponent>(ICON_MDI_VECTOR_SQUARE " Mesh", entity, [&](MeshComponent& component)
		{
			if(ImGui::Button("Import"))
			{
				eastl::string filepath = FileDialogs::OpenFile("Mesh (*.obj)\0*.obj\0(*.fbx)\0*.fbx\0");
				if (!filepath.empty())
				{
					component.Filepath = filepath;
					WriteMesh(filepath.c_str());
					return;
				}
			}
			
			if(ImGui::Button(component.MeshGeometry ? component.MeshGeometry->GetName() : "null"))
			{
				eastl::string filepath = FileDialogs::OpenFile("Mesh (*.assbin)\0*.assbin\0(*.obj)\0*.obj\0(*.fbx)\0*.fbx\0");
				if (!filepath.empty())
				{
					component.Filepath = filepath;
					component.MeshGeometry = CreateRef<Mesh>(filepath.c_str());
					return;
				}
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const char* path = (const char*)payload->Data;
					component.Filepath = path;
					component.MeshGeometry = CreateRef<Mesh>(path);

					ImGui::EndDragDropTarget();
					return;
				}
				ImGui::EndDragDropTarget();
			}

			if (component.MeshGeometry)
			{
				UI::BeginProperties();
				UI::Property("Submesh Index", component.SubmeshIndex, 0, component.MeshGeometry->GetSubmeshCount() - 1);
				const char* cullModeTypeStrings[] = { "Front", "Back", "Double Sided" };
				int cullMode = (int) component.CullMode;
				if (UI::Property("Cull Mode", cullMode, cullModeTypeStrings, 3))
					component.CullMode = (MeshComponent::CullModeType) cullMode;
				UI::EndProperties();

				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
					| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

				if (ImGui::TreeNodeEx("Material Properties", treeNodeFlags))
				{
					DrawMaterialProperties(component.MeshGeometry->GetSubmesh(component.SubmeshIndex).Mat);
					ImGui::TreePop();
				}
			}
		});

		DrawComponent<SkyLightComponent>(ICON_MDI_EARTH " Sky Light", entity, [](SkyLightComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Intensity", component.Intensity);
			UI::Property("Rotation", component.Rotation, 0.0f, 360.0f);
			UI::Property("Texture", component.Texture, component.Texture == nullptr ? 0 : component.Texture->GetHRDRendererID());
			UI::EndProperties();
		});

		DrawComponent<LightComponent>(ICON_MDI_LIGHTBULB " Light", entity, [](LightComponent& component)
		{
			UI::BeginProperties();
			const char* lightTypeStrings[] = { "Directional", "Point", "Spot" };
			int lightType = (int) component.Type;
			if (UI::Property("Light Type", lightType, lightTypeStrings, 3))
				component.Type = (LightComponent::LightType) lightType;

			if (UI::Property("Use color temprature mode", component.UseColorTempratureMode))
			{
				if (component.UseColorTempratureMode)
					component.SetTemprature(component.GetTemprature());
			}

			if (component.UseColorTempratureMode)
			{
				int temp = component.GetTemprature();
				if (UI::Property("Tempratur (K)", temp, 1000, 40000))
					component.SetTemprature(temp);
			}
			else
			{
				UI::PropertyColor3("Color", component.Color);
			}

			if (UI::Property("Intensity", component.Intensity))
			{
				if (component.Intensity < 0.0f)
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
				if (component.Range < 0.1f)
					component.Range = 0.1f;

				UI::Property("Outer Cut-Off Angle", component.OuterCutOffAngle, 1.0f, 90.0f);
				if (component.OuterCutOffAngle < component.CutOffAngle)
					component.CutOffAngle = component.OuterCutOffAngle;

				UI::Property("Cut-Off Angle", component.CutOffAngle, 1.0f, 90.0f);
				if (component.CutOffAngle > component.OuterCutOffAngle)
					component.OuterCutOffAngle = component.CutOffAngle;
			}
			else
			{
				const char* shadowQualityTypeStrings[] = { "Hard", "Soft", "Ultra Soft" };
				int shadowQualityType = (int) component.ShadowQuality;
				if (UI::Property("Shadow Quality Type", shadowQualityType, shadowQualityTypeStrings, 3))
					component.ShadowQuality = (LightComponent::ShadowQualityType) shadowQualityType;

				uint32_t textureID = component.ShadowMapFramebuffer->GetDepthAttachmentRendererID();
				ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ 256, 256 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			}

			UI::EndProperties();
		});

		DrawComponent<Rigidbody2DComponent>(ICON_MDI_SOCCER " Rigidbody 2D", entity, [](Rigidbody2DComponent& component)
		{
			UI::BeginProperties();

			const char* bodyTypeStrings[] = { "Static", "Kinematic", "Dynamic" };
			int bodyType = (int) component.Type;
			if (UI::Property("Body Type", bodyType, bodyTypeStrings, 3))
				component.Type = (Rigidbody2DComponent::BodyType)bodyType;

			if (component.Type == Rigidbody2DComponent::BodyType::Dynamic)
			{
				UI::Property("Auto Mass", component.AutoMass);
				if (!component.AutoMass)
					UI::Property("Mass", component.Mass, 0.1f, 0.01f, 100.0f);
				UI::Property("Linear Drag", component.LinearDrag);
				UI::Property("Angular Drag", component.AngularDrag);
				UI::Property("Gravity Scale", component.GravityScale);
				UI::Property("Allow Sleep", component.AllowSleep);
				UI::Property("Awake", component.Awake);
				UI::Property("Continuous", component.Continuous);
				UI::Property("Freeze Rotation", component.FreezeRotation);
			}

			UI::EndProperties();
		});

		DrawComponent<BoxCollider2DComponent>(ICON_MDI_CHECKBOX_BLANK_OUTLINE " Box Collider 2D", entity, [](BoxCollider2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Size", component.Size);
			UI::Property("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction);
			UI::Property("Restitution", component.Restitution);
			UI::Property("Restitution Threshold", component.RestitutionThreshold);
			UI::EndProperties();
		});

		DrawComponent<CircleCollider2DComponent>(ICON_MDI_CIRCLE_OUTLINE " Circle Collider 2D", entity, [](CircleCollider2DComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Radius", component.Radius);
			UI::Property("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction);
			UI::Property("Restitution", component.Restitution);
			UI::Property("Restitution Threshold", component.RestitutionThreshold);
			UI::EndProperties();
		});

		DrawComponent<ScriptComponent>(ICON_MDI_POUND_BOX " Script", entity, [](ScriptComponent& component)
		{
			UI::BeginProperties();
			bool found = ScriptEngine::HasClass(component.ClassName.c_str());
			ImGui::PushStyleColor(ImGuiCol_Text, { found ? 0.2f : 0.8f, found ? 0.8f : 0.2f, 0.2f, 1.0f});
			UI::Property("Name", component.ClassName);
			ImGui::PopStyleColor();
			UI::EndProperties();

			if (found)
			{
				if (!component.Handle)
					component.Handle = ScriptEngine::MakeReference(component.ClassName.c_str());

				ImGui::PushID(component.Handle);
				DrawFields(component);
				ImGui::PopID();
			}
			else if (component.Handle)
			{
				ScriptEngine::ReleaseObjectReference(component.Handle);
				component.Handle = nullptr;
			}
		});

		ImGui::Spacing();

		/////////////////////////////////////////////////////////////
		//ADD COMPONENT//////////////////////////////////////////////
		/////////////////////////////////////////////////////////////
		ImVec2 addComponentButtonSize = ImVec2(150.0f, 30.0f);
		{
			ImVec2 pos = ImGui::GetCursorPos();
			ImVec2 available = ImGui::GetContentRegionAvail();
			pos = ImVec2(available.x * 0.5f - addComponentButtonSize.x * 0.5f, pos.y + ImGui::GetFrameHeight() * 0.5f);
			ImGui::SetCursorPos(pos);
		}

		if (ImGui::Button(ICON_MDI_PLUS "Add Component", addComponentButtonSize))
			ImGui::OpenPopup("Add Component");

		if (ImGui::BeginPopup("Add Component"))
		{
			if (!entity.HasComponent<CameraComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_CAMERA " Camera"))
				{
					entity.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_IMAGE_SIZE_SELECT_ACTUAL " Sprite Renderer"))
				{
					entity.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<MeshComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_VECTOR_SQUARE " Mesh"))
				{
					entity.AddComponent<MeshComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<SkyLightComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_EARTH " Sky Light"))
				{
					entity.AddComponent<SkyLightComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<LightComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_LIGHTBULB " Light"))
				{
					entity.AddComponent<LightComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<Rigidbody2DComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_SOCCER " Rigidbody 2D"))
				{
					entity.AddComponent<Rigidbody2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_CHECKBOX_BLANK_OUTLINE " Box Collider 2D"))
				{
					entity.AddComponent<BoxCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_CIRCLE_OUTLINE " Circle Collider 2D"))
				{
					entity.AddComponent<CircleCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<ScriptComponent>())
			{
				if (ImGui::MenuItem(ICON_MDI_POUND_BOX " Script"))
				{
					entity.AddComponent<ScriptComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			
			ImGui::EndPopup();
		}
	}
}
