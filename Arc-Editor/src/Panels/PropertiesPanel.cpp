#include "PropertiesPanel.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Arc/Utils/PlatformUtils.h"
#include "../Utils/UI.h"

namespace ArcEngine
{
	uint32_t PropertiesPanel::s_ID = 0;
	
	void PropertiesPanel::OnImGuiRender()
	{
		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin(m_ID.c_str(), &m_Showing);

		if(m_Context && m_Context.GetScene())
			DrawComponents(m_Context);
		
		ImGui::End();
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
	
		static void DrawFloatControl(const std::string& label, float* value, float columnWidth = 100.0f)
	{
		ImGui::PushID(label.c_str());
		
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::DragFloat("##value", value, 0.1f);

		ImGui::Columns(1);
		ImGui::PopID();
	}

	static void DrawTextureCubemapButton(const std::string& label, Ref<TextureCubemap>& texture, uint32_t overrideTextureID = 0)
	{
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
				texture = TextureCubemap::Create(filepath);
			}
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
				texture = Texture2D::Create(filepath);
			}
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

	static std::vector<Ref<Texture2D>> s_TextureCache;
	std::vector<Ref<Texture2D>> LoadMaterialTextures(aiMaterial *mat, aiTextureType type, const char* filepath)
    {
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
			meshComponent.UseAlbedoMap = true;
		}

		if (normalMaps.size() > 0)
		{
			meshComponent.NormalMap = normalMaps[0];
			meshComponent.UseNormalMap = true;
		}
		else if (heightMaps.size() > 0)
		{
			meshComponent.NormalMap = heightMaps[0];
			meshComponent.UseNormalMap = true;
		}

		if (emissiveMaps.size() > 0)
		{
			meshComponent.EmissiveMap = emissiveMaps[0];
			meshComponent.UseEmissiveMap = true;
		}
	}

	static void ProcessNode(aiNode *node, const aiScene *scene, Entity entity, const char* filepath)
	{
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
		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 80.0f);
		const uint32_t meshImportFlags =
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords |
	        aiProcess_OptimizeMeshes |
			aiProcess_JoinIdenticalVertices |
			aiProcess_GlobalScale |
			aiProcess_ImproveCacheLocality |
	        aiProcess_ValidateDataStructure;

		const aiScene *scene = importer.ReadFile(filepath, meshImportFlags);
 
		ARC_CORE_ASSERT(scene, importer.GetErrorString());

		if (rootEntity.HasComponent<MeshComponent>())
			rootEntity.RemoveComponent<MeshComponent>();
		ProcessNode(scene->mRootNode, scene, rootEntity, filepath);
	}

	void PropertiesPanel::DrawComponents(Entity entity)
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

		if (ImGui::Button(m_Locked ? "Unlock" : "Lock"))
			m_Locked = !m_Locked;

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

			if (ImGui::MenuItem("Mesh"))
			{
				if (!entity.HasComponent<MeshComponent>())
					entity.AddComponent<MeshComponent>();
				else
					ARC_CORE_WARN("This entity already has the Mesh Component!");
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sky Light"))
			{
				if (!entity.HasComponent<SkyLightComponent>())
					entity.AddComponent<SkyLightComponent>();
				else
					ARC_CORE_WARN("This entity already has the Sky Light Component!");
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Light"))
			{
				if (!entity.HasComponent<LightComponent>())
					entity.AddComponent<LightComponent>();
				else
					ARC_CORE_WARN("This entity already has the Light Component!");
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

			DrawTexture2DButton("Texture", component.Texture);

			ImGui::Spacing();
			
			DrawFloatControl("Tiling Factor", &component.TilingFactor, 200);
		});

		DrawComponent<MeshComponent>("Mesh", entity, [&](MeshComponent& component)
		{
			ImGui::Text("Mesh");
			const ImVec2 buttonSize = { 60, 20 };

			ImGui::Text(component.Filepath.c_str());

			ImGui::SameLine(ImGui::GetWindowWidth() - buttonSize.x - 10);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
			if(ImGui::Button("...", buttonSize))
			{
				std::string filepath = FileDialogs::OpenFile("Mesh (*.obj)\0*.obj\0(*.fbx)\0*.fbx\0(*.gltf)\0*.gltf\0");
				if (!filepath.empty())
				{
					component.Filepath = filepath;
					LoadMesh(filepath.c_str(), entity);
					
					// TEMP
					ImGui::PopStyleColor(3);
					return;
				}
			}
			ImGui::PopStyleColor(3);

			const char* cullModeTypeStrings[] = { "Front", "Back", "Double Sided" };
			const char* currentCullModeTypeStrings = cullModeTypeStrings[(int)component.CullMode];
			if(ImGui::BeginCombo("Cull Mode", currentCullModeTypeStrings))
			{
				for (int i = 0; i < 3; i++)
				{
					bool isSelected = currentCullModeTypeStrings == cullModeTypeStrings[i];
					if(ImGui::Selectable(cullModeTypeStrings[i], isSelected))
					{
						currentCullModeTypeStrings = cullModeTypeStrings[i];
						component.CullMode = (MeshComponent::CullModeType)i;
					}

					if(isSelected)
						ImGui::SetItemDefaultFocus();
				}
				
				ImGui::EndCombo();
			}

			ImGui::ColorEdit4("Albedo Color", glm::value_ptr(component.AlbedoColor));
			ImGui::SliderFloat("Metallic", &component.Metallic, 0.0f, 1.0f);
			ImGui::SliderFloat("Roughness", &component.Roughness, 0.0f, 1.0f);
			ImGui::ColorEdit3("Emissive Color", glm::value_ptr(component.EmissiveParams));
			ImGui::DragFloat("Emissive Intensity", &component.EmissiveParams.w);
			
			ImGui::Checkbox("##UseAlbedoMap", &component.UseAlbedoMap);
			DrawTexture2DButton("AlbedoMap", component.AlbedoMap);
			ImGui::Checkbox("##UseNormalMap", &component.UseNormalMap);
			DrawTexture2DButton("NormalMap", component.NormalMap);
			ImGui::Checkbox("##UseMetallicMap", &component.UseMRAMap);
			DrawTexture2DButton("MetallicMap", component.MRAMap);
			ImGui::Checkbox("##UseEmissiveMap", &component.UseEmissiveMap);
			DrawTexture2DButton("EmissiveMap", component.EmissiveMap);
			
		}, false);

		DrawComponent<SkyLightComponent>("Sky Light", entity, [](SkyLightComponent& component)
		{
			DrawFloatControl("Intensity", &component.Intensity);
			
			UI::BeginPropertyGrid();
			UI::Property("Rotation", component.Rotation, 0.0f, 360.0f);
			UI::EndPropertyGrid();

			const uint32_t id = component.Texture == nullptr ? 0 : component.Texture->GetHRDRendererID();
			DrawTextureCubemapButton("Texture", component.Texture, id);

			ImGui::Spacing();
		});

		DrawComponent<LightComponent>("Light", entity, [](LightComponent& component)
		{
			UI::PushID();

			const char* lightTypeStrings[] = { "Directional", "Point", "Spot" };
			const char* currentLightTypeString = lightTypeStrings[(int)component.Type];
			UI::BeginPropertyGrid();
			UI::Property("LightType");
			ImGui::NextColumn();
			if(ImGui::BeginCombo("##LightType", currentLightTypeString))
			{
				for (int i = 0; i < 3; i++)
				{
					bool isSelected = currentLightTypeString == lightTypeStrings[i];
					if(ImGui::Selectable(lightTypeStrings[i], isSelected))
					{
						currentLightTypeString = lightTypeStrings[i];
						component.Type = (LightComponent::LightType)i;
					}

					if(isSelected)
						ImGui::SetItemDefaultFocus();
				}
				
				ImGui::EndCombo();
			}
			UI::EndPropertyGrid();

			UI::BeginPropertyGrid();
			if (UI::Property("Use color temprature mode", component.UseColorTempratureMode))
			{
				if (component.UseColorTempratureMode)
					component.SetTemprature(component.GetTemprature());
			}
			UI::EndPropertyGrid();

			if (component.UseColorTempratureMode)
			{
				UI::BeginPropertyGrid();
				int temp = component.GetTemprature();
				if (UI::Property("Tempratur (K)", temp, 1000, 40000))
					component.SetTemprature(temp);
				UI::EndPropertyGrid();
			}
			else
			{
				UI::BeginPropertyGrid();
				UI::Property("Color");
				ImGui::NextColumn();
				ImGui::ColorEdit3("##Color", glm::value_ptr(component.Color));
				UI::EndPropertyGrid();
			}

			UI::BeginPropertyGrid();
			if (UI::Property("Intensity", component.Intensity))
			{
				if (component.Intensity < 0.0f)
					component.Intensity = 0.0f;
			}
			UI::EndPropertyGrid();

			ImGui::Spacing();

			if (component.Type == LightComponent::LightType::Point)
			{
				UI::BeginPropertyGrid();
				UI::Property("Range", component.Range);
				UI::EndPropertyGrid();
			}
			else if (component.Type == LightComponent::LightType::Spot)
			{
				UI::BeginPropertyGrid();
				UI::Property("Range", component.Range);
				if (component.Range < 0.1f)
					component.Range = 0.1f;
				UI::EndPropertyGrid();

				UI::BeginPropertyGrid();
				UI::Property("Outer Cut-Off Angle", component.OuterCutOffAngle, 1.0f, 90.0f);
				if (component.OuterCutOffAngle < component.CutOffAngle)
					component.CutOffAngle = component.OuterCutOffAngle;
				UI::EndPropertyGrid();

				UI::BeginPropertyGrid();
				UI::Property("Cut-Off Angle", component.CutOffAngle, 1.0f, 90.0f);
				if (component.CutOffAngle > component.OuterCutOffAngle)
					component.OuterCutOffAngle = component.CutOffAngle;
				UI::EndPropertyGrid();
			}
			else
			{
				uint32_t textureID = component.ShadowMapFramebuffer->GetDepthAttachmentRendererID();
				ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ 256, 256 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			}

			UI::PopID();
		});
	}
}
