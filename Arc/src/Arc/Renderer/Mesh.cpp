#include "arcpch.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Arc/Core/AssetManager.h"
#include "Arc/Core/QueueSystem.h"
#include "Arc/Renderer/Material.h"
#include "Arc/Renderer/Shader.h"
#include "Arc/Renderer/VertexArray.h"
#include "Arc/Utils/StringUtils.h"

namespace ArcEngine
{
	struct Vertex
	{
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec2 TexCoord = glm::vec2(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f);
	};
}

namespace ArcEngine
{
	Mesh::Mesh(const char* filepath)
	{
		ARC_PROFILE_SCOPE();

		Load(filepath);
	}

	void Mesh::Load(const char* filepath)
	{
		ARC_PROFILE_SCOPE();

		std::filesystem::path path = filepath;

		if (!std::filesystem::exists(path))
			return;

		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 80.0f);
		uint32_t meshImportFlags = 0;

		eastl::string filePath = eastl::string(filepath);
		auto lastDot = filePath.find_last_of(".");
		eastl::string name = filePath.substr(lastDot + 1, filePath.size() - lastDot);
		if (name != "assbin")
		{
			meshImportFlags |=
				aiProcess_MakeLeftHanded |					// for D3D
				aiProcess_FlipUVs |							// for D3D
				aiProcess_Triangulate |
				aiProcess_SortByPType |
				aiProcess_GenNormals |
				aiProcess_GenUVCoords |
				aiProcess_TransformUVCoords |
				//aiProcess_OptimizeGraph |
				aiProcess_OptimizeMeshes |
				aiProcess_JoinIdenticalVertices |
				aiProcess_ImproveCacheLocality |
				aiProcess_PreTransformVertices |
				aiProcess_LimitBoneWeights |
				aiProcess_ValidateDataStructure |
				aiProcess_GlobalScale;
		}

		const aiScene* scene = importer.ReadFile(filepath, meshImportFlags);

		if (!scene)
		{
			ARC_CORE_ERROR("Could not import the file: {0}. Error: {1}", filepath, importer.GetErrorString());
			return;
		}

		//JobSystem::Wait();
		ProcessNode(scene->mRootNode, scene, filepath);
		//QueueSystem::CompleteAllWork(nullptr);
		//JobSystem::Wait();
		m_Name = StringUtils::GetName(filepath);

		m_Filepath = filepath;
		m_Name = StringUtils::GetName(filepath);
	}

	Submesh& Mesh::GetSubmesh(size_t index)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(index < m_Submeshes.size(), "Submesh index out of bounds");

		return m_Submeshes[index];
	}

	void Mesh::ProcessNode(const aiNode* node, const aiScene* scene, const char* filepath)
	{
		ARC_PROFILE_SCOPE();

		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			/*
			struct ProcessMeshData
			{
				Mesh* meshClass;
				
				unsigned int meshIndex;
				const aiNode* node;
				const aiScene* scene;
				const char* filepath;
			};
			ProcessMeshData meshData = { this, i, node, scene, filepath };
			QueueSystem::AddEntry(nullptr, [](WorkQueue* queue, void* data)
			{
				ProcessMeshData meshData = *(ProcessMeshData*)data;
				aiMesh* mesh = meshData.scene->mMeshes[meshData.node->mMeshes[meshData.meshIndex]];
				meshData.meshClass->ProcessMesh(mesh, meshData.scene, meshData.filepath, meshData.node->mName.C_Str());
			}, &meshData);
			*/
			//JobSystem::Execute([mesh, scene, filepath, node, this]() { ProcessMesh(mesh, scene, filepath, node->mName.C_Str()); });
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene, filepath, node->mName.C_Str());
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene, filepath);
	}

	eastl::vector<Ref<Texture2D>> LoadMaterialTextures(const aiMaterial* mat, aiTextureType type, const std::filesystem::path& filepath)
	{
		ARC_PROFILE_SCOPE();

		std::filesystem::path path = filepath.parent_path();
		eastl::vector<Ref<Texture2D>> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			std::filesystem::path texturePath = path / str.C_Str();
			std::string texturePathString = texturePath.string();
			Ref<Texture2D> texture = AssetManager::GetTexture2D(texturePathString.c_str());
			textures.emplace_back(texture);
		}
		return textures;
	}

	void Mesh::ProcessMesh(aiMesh* mesh, const aiScene* scene, const char* filepath, const char* nodeName)
	{
		ARC_PROFILE_SCOPE();

		eastl::vector<Vertex> vertices;
		vertices.reserve(mesh->mNumVertices);
		eastl::vector<uint32_t> indices;

		for (size_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			const auto& vertexPosition = mesh->mVertices[i];
			vertex.Position = { vertexPosition.x, vertexPosition.y, vertexPosition.z };

			if (mesh->mTextureCoords[0])
				vertex.TexCoord = glm::vec2(static_cast<float>(mesh->mTextureCoords[0][i].x), static_cast<float>(mesh->mTextureCoords[0][i].y));

			const auto& normal = mesh->mNormals[i];
			vertex.Normal = { normal.x, normal.y, normal.z };

			vertices.emplace_back(vertex);
		}

		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			indices.reserve(face.mNumIndices);
			for (size_t j = 0; j < face.mNumIndices; j++)
				indices.emplace_back(face.mIndices[j]);
		}

		Ref<VertexArray> vertexArray = VertexArray::Create();

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(reinterpret_cast<float*>(vertices.data()), static_cast<uint32_t>(sizeof(Vertex) * vertices.size()), sizeof(Vertex));
		vertexArray->AddVertexBuffer(vertexBuffer);

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
		vertexArray->SetIndexBuffer(indexBuffer);

		std::lock_guard lock(m_SubmeshesMutex);

		const Submesh& submesh = m_Submeshes.emplace_back(nodeName, CreateRef<Material>(), vertexArray);

		const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		eastl::vector<Ref<Texture2D>> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, filepath);
		eastl::vector<Ref<Texture2D>> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, filepath);
		eastl::vector<Ref<Texture2D>> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, filepath);
		eastl::vector<Ref<Texture2D>> emissiveMaps = LoadMaterialTextures(material, aiTextureType_EMISSIVE, filepath);

		const eastl::vector<MaterialProperty>& materialProperties = submesh.Mat->GetProperties();

		bool normalMapApplied = false;
		for (const auto& property : materialProperties)
		{
			const auto& name = property.Name;
			
			if (property.Type == MaterialPropertyType::Texture2D ||	property.Type == MaterialPropertyType::Texture2DBindless)
			{
				if (!diffuseMaps.empty() &&
					(name.find("albedo") != eastl::string::npos || name.find("Albedo") != eastl::string::npos ||
						name.find("diff") != eastl::string::npos || name.find("Diff") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(property.Name, diffuseMaps[0]);
				}

				if (!normalMaps.empty() &&
					(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
						name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(property.Name, normalMaps[0]);
					normalMapApplied = true;
				}
				else if (!heightMaps.empty() &&
					(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
						name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(property.Name, heightMaps[0]);
					normalMapApplied = true;
				}

				if (!emissiveMaps.empty() &&
					(name.find("emissi") != eastl::string::npos || name.find("Emissi") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(property.Name, emissiveMaps[0]);
				}
			}

			if (property.Type == MaterialPropertyType::Bool && normalMapApplied &&
				(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
					name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
			{
				submesh.Mat->SetData(property.Name, 1);
			}
		}
	}
}
