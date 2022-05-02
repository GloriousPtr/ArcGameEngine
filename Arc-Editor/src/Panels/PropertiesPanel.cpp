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
		OPTICK_EVENT();

		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin(m_ID.c_str(), &m_Showing);

		if(m_Context && m_Context.GetScene())
			DrawComponents(m_Context);
		
		ImGui::End();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, const bool removable = true)
	{
		OPTICK_EVENT();

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
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.75f);
				if(ImGui::Button(ICON_MDI_SETTINGS, ImVec2{ lineHeight, lineHeight }))
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
				OPTICK_EVENT("UI Function");

				uiFunction(component);
				ImGui::TreePop();
			}

			if(removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	static void DrawTextureCubemapButton(const std::string& label, Ref<TextureCubemap>& texture, uint32_t overrideTextureID = 0)
	{
		OPTICK_EVENT();

		ImGui::PushID(label.c_str());

		uint32_t id = overrideTextureID;
		
		if (id == 0)
			id = texture == nullptr ? 0 : texture->GetHRDRendererID();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		
		ImGui::Text(label.c_str());
		const ImVec2 buttonSize = { 80, 80 };
		ImGui::SameLine(ImGui::GetWindowWidth() - buttonSize.x - 30);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		if(ImGui::ImageButton((ImTextureID)id, buttonSize, { 1, 1 }, { 0, 0 }, 0))
		{
			std::string filepath = FileDialogs::OpenFile("Texture (*.hdr)\0*.hdr\0");
			if (!filepath.empty())
			{
				texture = AssetManager::GetTextureCubemap(filepath);
			}
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				texture = AssetManager::GetTextureCubemap(path);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		if(ImGui::Button("x", { buttonSize.x / 4, buttonSize.y } ))
			texture = nullptr;
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		ImGui::PopID();
	}

	static void DrawTexture2DButton(const std::string& label, Ref<Texture2D>& texture, uint32_t overrideTextureID = 0)
	{
		OPTICK_EVENT();

		ImGui::PushID(label.c_str());

		uint32_t id = overrideTextureID;
		
		if (id == 0)
			id = texture == nullptr ? 0 : texture->GetRendererID();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		
		ImGui::Text(label.c_str());
		const ImVec2 buttonSize = { 80, 80 };
		ImGui::SameLine(ImGui::GetWindowWidth() - buttonSize.x - 30);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
		if(ImGui::ImageButton((ImTextureID)id, buttonSize, { 1, 1 }, { 0, 0 }, 0))
		{
			std::string filepath = FileDialogs::OpenFile("Texture (*.png)\0*.png\0(*.jpg)\0*.jpg\0");
			if (!filepath.empty())
			{
				texture = AssetManager::GetTexture2D(filepath);
			}
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				texture = AssetManager::GetTexture2D(path);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		if(ImGui::Button("x", { buttonSize.x / 4, buttonSize.y } ))
			texture = nullptr;
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		ImGui::PopID();
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		OPTICK_EVENT();

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

	static void DrawFields(ScriptComponent& component)
	{
		// Public Fields
		std::unordered_map<std::string, Field>* fieldMap = ScriptEngine::GetFields(component.ClassName.c_str());
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
						std::string& value = field.GetValueString(component.Handle);
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
	}

	static std::vector<Ref<Texture2D>> s_TextureCache;
	std::vector<Ref<Texture2D>> LoadMaterialTextures(aiMaterial *mat, aiTextureType type, const char* filepath)
    {
		OPTICK_EVENT();

		std::string path = std::string(filepath);
		std::string dir = path.substr(0, path.find_last_of('\\'));
        std::vector<Ref<Texture2D>> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
			aiString str;
			mat->GetTexture(type, i, &str);

			bool skip = false;
			for(unsigned int j = 0; j < s_TextureCache.size(); j++)
            {
                if(std::strcmp(s_TextureCache[j]->GetPath().c_str(), std::string(dir + '\\' + str.C_Str()).c_str()) == 0)
                {
                    textures.push_back(s_TextureCache[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }

			if (!skip)
			{
				Ref<Texture2D> texture = Texture2D::Create(dir + '\\' + str.C_Str());
				textures.push_back(texture);
				s_TextureCache.push_back(texture);
			}
        }
        return textures;
    }

	static void ProcessMesh(aiMesh *mesh, const aiScene *scene, Entity e, const char* filepath)
	{
		OPTICK_EVENT();

		std::vector<float> vertices;
		size_t count = mesh->mNumVertices * 14;
		vertices.reserve(count);

		std::vector<uint32_t> indices;
		AABB boundingBox;
		for(size_t i = 0; i < mesh->mNumVertices; i++)
		{
			auto& vertexPosition = mesh->mVertices[i];
			vertices.push_back(vertexPosition.x);
			vertices.push_back(vertexPosition.y);
			vertices.push_back(vertexPosition.z);

			boundingBox.Min.x = glm::min(vertexPosition.x, boundingBox.Min.x);
			boundingBox.Min.y = glm::min(vertexPosition.y, boundingBox.Min.y);
			boundingBox.Min.z = glm::min(vertexPosition.z, boundingBox.Min.z);
			boundingBox.Max.x = glm::max(vertexPosition.x, boundingBox.Max.x);
			boundingBox.Max.y = glm::max(vertexPosition.y, boundingBox.Max.y);
			boundingBox.Max.z = glm::max(vertexPosition.z, boundingBox.Max.z);

			if(mesh->mTextureCoords[0])
            {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            }
            else
			{
				vertices.push_back(0.0f);
				vertices.push_back(0.0f);
			}
			
			auto& normal = mesh->mNormals[i];
			vertices.push_back(normal.x);
			vertices.push_back(normal.y);
			vertices.push_back(normal.z);

			if (mesh->mTangents)
			{
				auto tangent = mesh->mTangents[i];
				vertices.push_back(tangent.x);
				vertices.push_back(tangent.y);
				vertices.push_back(tangent.z);

				auto bitangent = mesh->mBitangents[i];
				vertices.push_back(bitangent.x);
				vertices.push_back(bitangent.y);
				vertices.push_back(bitangent.z);
			}
			else
			{
				size_t index = i / 3;
				// Shortcuts for vertices
				auto& v0 = mesh->mVertices[index + 0];
				auto& v1 = mesh->mVertices[index + 1];
				auto& v2 = mesh->mVertices[index + 2];

				// Shortcuts for UVs
				auto& uv0 = mesh->mTextureCoords[0][index + 0];
				auto& uv1 = mesh->mTextureCoords[0][index + 1];
				auto& uv2 = mesh->mTextureCoords[0][index + 2];

				// Edges of the triangle : position delta
				auto deltaPos1 = v1 - v0;
				auto deltaPos2 = v2 - v0;

				// UV delta
				auto deltaUV1 = uv1 - uv0;
				auto deltaUV2 = uv2 - uv0;

				float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				auto tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
				auto bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

				vertices.push_back(tangent.x);
				vertices.push_back(tangent.y);
				vertices.push_back(tangent.z);
				vertices.push_back(bitangent.x);
				vertices.push_back(bitangent.y);
				vertices.push_back(bitangent.z);
			}
		}

		for(size_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			indices.reserve(face.mNumIndices);
			for(size_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		Ref<VertexArray> vertexArray = VertexArray::Create();

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(&(vertices[0]), (sizeof(float) * vertices.size()));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Bitangent" },
		});
		vertexArray->AddVertexBuffer(vertexBuffer);

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(&(indices[0]), indices.size());
		vertexArray->SetIndexBuffer(indexBuffer);

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Ref<Texture2D>> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, filepath);
		std::vector<Ref<Texture2D>> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, filepath);
		std::vector<Ref<Texture2D>> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, filepath);
		std::vector<Ref<Texture2D>> emissiveMaps = LoadMaterialTextures(material, aiTextureType_EMISSIVE, filepath);

		if (!e.HasComponent<MeshComponent>())
			e.AddComponent<MeshComponent>();

		auto& meshComponent = e.GetComponent<MeshComponent>();
		
		meshComponent.Filepath = filepath;
		meshComponent.VertexArray = vertexArray;
		meshComponent.BoundingBox = boundingBox;

		if (diffuseMaps.size() > 0)
		{
			meshComponent.AlbedoMap = diffuseMaps[0];
			meshComponent.UseMaps.x = 1.0f;
		}

		if (normalMaps.size() > 0)
		{
			meshComponent.NormalMap = normalMaps[0];
			meshComponent.UseMaps.y = 1.0f;
		}
		else if (heightMaps.size() > 0)
		{
			meshComponent.NormalMap = heightMaps[0];
			meshComponent.UseMaps.y = 1.0f;
		}

		if (emissiveMaps.size() > 0)
		{
			meshComponent.EmissiveMap = emissiveMaps[0];
			meshComponent.UseMaps.w = 1.0f;
		}
	}

	static void ProcessNode(aiNode *node, const aiScene *scene, Entity entity, const char* filepath)
	{
		OPTICK_EVENT();

		for(unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene, entity, filepath);
		}

		for(unsigned int i = 0; i < node->mNumChildren; i++)
		{
			std::string name = node->mChildren[i]->mName.C_Str();
			Scene* s = entity.GetScene();
			if (s == nullptr)
				return;

			Entity e = s->CreateEntity(name);
			e.SetParent(entity);
			ProcessNode(node->mChildren[i], scene, e, filepath);
		}
	}
	
	static void LoadMesh(const char* filepath, Entity rootEntity)
	{
		OPTICK_EVENT();

		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 80.0f);
		uint32_t meshImportFlags = 0;
		
		std::string filePath = std::string(filepath);
		auto lastDot = filePath.find_last_of(".");
		std::string name = filePath.substr(lastDot + 1, filePath.size() - lastDot);
		if (name != "assbin")
		{
			meshImportFlags |=
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
		}

		const aiScene *scene = importer.ReadFile(filepath, meshImportFlags);

		if (!scene)
		{
			ARC_CORE_ERROR("Could not import the file: {0}. Error: {1}", filepath, importer.GetErrorString());
			return;
		}

		if (rootEntity.HasComponent<MeshComponent>())
			rootEntity.RemoveComponent<MeshComponent>();
		ProcessNode(scene->mRootNode, scene, rootEntity, filepath);
	}

	static void WriteMesh(const std::string& filepath)
	{
		OPTICK_EVENT();

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

		const aiScene *scene = importer.ReadFile(filepath, meshImportFlags);
		if (!scene)
		{
			ARC_CORE_ERROR("Could not import the file: {0}. Error: {1}", filepath, importer.GetErrorString());
			return;
		}

		Assimp::Exporter exporter;
		const aiExportFormatDesc* format = exporter.GetExportFormatDescription(0);
		auto lastDot = filepath.find_last_of(".");
		std::string path = filepath.substr(0, lastDot) + ".assbin";
		aiReturn ret = exporter.Export(scene, format->id, path.c_str(), meshImportFlags);
		if (ret != aiReturn_SUCCESS)
		{
			ARC_CORE_ERROR("Could not import the file: {0}. Error: {1}", filepath, exporter.GetErrorString());
			return;
		}
	}

	void PropertiesPanel::DrawComponents(Entity entity)
	{
		OPTICK_EVENT();

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
			DrawVec3Control("Translation", component.Translation);
			
			glm::vec3 rotation = glm::degrees(component.Rotation);
			DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);

			DrawVec3Control("Scale", component.Scale, 1.0f);
		}, false);

		DrawComponent<CameraComponent>(ICON_MDI_CAMERA " Camera", entity, [](CameraComponent& component)
		{
			auto& camera = component.Camera;

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
		});
		
		DrawComponent<SpriteRendererComponent>(ICON_MDI_IMAGE_SIZE_SELECT_ACTUAL " Sprite Renderer", entity, [](SpriteRendererComponent& component)
		{
			UI::PropertyColor4("Color", component.Color);

			DrawTexture2DButton("Texture", component.Texture);

			ImGui::Spacing();
			
			UI::Property("Tiling Factor", component.TilingFactor, 200);
		});

		DrawComponent<MeshComponent>(ICON_MDI_VECTOR_SQUARE " Mesh", entity, [&](MeshComponent& component)
		{
			ImGui::Text("Mesh");

			if(ImGui::Button("Import"))
			{
				std::string filepath = FileDialogs::OpenFile("Mesh (*.obj)\0*.obj\0(*.fbx)\0*.fbx\0");
				if (!filepath.empty())
				{
					component.Filepath = filepath;
					WriteMesh(filepath.c_str());
					return;
				}
			}
			if(ImGui::Button("Load"))
			{
				std::string filepath = FileDialogs::OpenFile("Mesh (*.assbin)\0*.assbin\0(*.obj)\0*.obj\0(*.fbx)\0*.fbx\0");
				if (!filepath.empty())
				{
					component.Filepath = filepath;
					LoadMesh(filepath.c_str(), entity);
					return;
				}
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const char* path = (const char*)payload->Data;
					LoadMesh(path, entity);

					ImGui::EndDragDropTarget();
					return;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Text(component.Filepath.c_str());

			const char* cullModeTypeStrings[] = { "Front", "Back", "Double Sided" };
			int cullMode = (int) component.CullMode;
			if (UI::Property("Cull Mode", cullMode, cullModeTypeStrings, 3))
				component.CullMode = (MeshComponent::CullModeType) cullMode;

			UI::PropertyColor4("Albedo Color", component.AlbedoColor);
			UI::Property("Metallic", component.MR.x, 0.0f, 1.0f);
			UI::Property("Roughness", component.MR.y, 0.0f, 1.0f);
			UI::PropertyColor4as3("Emissive Color", component.EmissiveParams);
			UI::Property("Emissive Intensity", component.EmissiveParams.w);
			
			bool usingMap = component.UseMaps.x;
			if (UI::Property("Use Albedo Map", usingMap))
				component.UseMaps.x = usingMap ? 1.0f : 0.0f;
			DrawTexture2DButton("Albedo Map", component.AlbedoMap);

			usingMap = component.UseMaps.y;
			if (UI::Property("UseNormalMap", usingMap))
				component.UseMaps.y = usingMap ? 1.0f : 0.0f;
			DrawTexture2DButton("Normal Map", component.NormalMap);
			
			usingMap = component.UseMaps.z;
			if (UI::Property("Use Metallic Map", usingMap))
				component.UseMaps.z = usingMap ? 1.0f : 0.0f;
			DrawTexture2DButton("Metallic Map", component.MRAMap);
			
			usingMap = component.UseMaps.w;
			if (UI::Property("Use Emissive Map", usingMap))
				component.UseMaps.w = usingMap ? 1.0f : 0.0f;
			DrawTexture2DButton("Emissive Map", component.EmissiveMap);
		});

		DrawComponent<SkyLightComponent>(ICON_MDI_EARTH " Sky Light", entity, [](SkyLightComponent& component)
		{
			UI::Property("Intensity", component.Intensity);
			UI::Property("Rotation", component.Rotation, 0.0f, 360.0f);
			const uint32_t id = component.Texture == nullptr ? 0 : component.Texture->GetHRDRendererID();
			DrawTextureCubemapButton("Texture", component.Texture, id);

			ImGui::Spacing();
		});

		DrawComponent<LightComponent>(ICON_MDI_LIGHTBULB " Light", entity, [](LightComponent& component)
		{
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
				uint32_t textureID = component.ShadowMapFramebuffer->GetDepthAttachmentRendererID();
				ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ 256, 256 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			}
		});

		DrawComponent<Rigidbody2DComponent>(ICON_MDI_SOCCER " Rigidbody 2D", entity, [](Rigidbody2DComponent& component)
		{
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
		});

		DrawComponent<BoxCollider2DComponent>(ICON_MDI_CHECKBOX_BLANK_OUTLINE " Box Collider 2D", entity, [](BoxCollider2DComponent& component)
		{
			UI::Property("Size", component.Size);
			UI::Property("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction);
			UI::Property("Restitution", component.Restitution);
			UI::Property("Restitution Threshold", component.RestitutionThreshold);
		});

		DrawComponent<CircleCollider2DComponent>(ICON_MDI_CIRCLE_OUTLINE " Circle Collider 2D", entity, [](CircleCollider2DComponent& component)
		{
			UI::Property("Radius", component.Radius);
			UI::Property("Offset", component.Offset);
			UI::Property("Density", component.Density);
			UI::Property("Friction", component.Friction);
			UI::Property("Restitution", component.Restitution);
			UI::Property("Restitution Threshold", component.RestitutionThreshold);
		});

		DrawComponent<ScriptComponent>(ICON_MDI_POUND_BOX " Script", entity, [](ScriptComponent& component)
		{
			bool found = ScriptEngine::HasClass(component.ClassName.c_str());
			ImGui::PushStyleColor(ImGuiCol_Text, { found ? 0.2f : 0.8f, found ? 0.8f : 0.2f, 0.2f, 1.0f});
			UI::Property("Name", component.ClassName);
			ImGui::PopStyleColor();

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

		ImGui::Separator();

		/////////////////////////////////////////////////////////////
		//ADD COMPONENT//////////////////////////////////////////////
		/////////////////////////////////////////////////////////////
		ImVec2 addComponentButtonSize = ImVec2(150.0f, 30.0f);
		{
			ImVec2 pos = ImGui::GetCursorPos();
			ImVec2 available = ImGui::GetContentRegionAvail();
			pos = ImVec2(available.x * 0.5f - addComponentButtonSize.x * 0.5f, pos.y + ImGui::GetFrameHeight());
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
