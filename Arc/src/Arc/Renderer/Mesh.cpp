#include "arcpch.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Arc/Core/AssetManager.h"
#include "Arc/Utils/StringUtils.h"

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

		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 80.0f);
		uint32_t meshImportFlags = 0;
		
		eastl::string filePath = eastl::string(filepath);
		auto lastDot = filePath.find_last_of(".");
		eastl::string name = filePath.substr(lastDot + 1, filePath.size() - lastDot);
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

		ProcessNode(scene->mRootNode, scene, filepath);
		m_Name = StringUtils::GetName(filepath);
	}

	Submesh& Mesh::GetSubmesh(uint32_t index)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(index < m_Submeshes.size(), "Submesh index out of bounds");

		return m_Submeshes[index];
	}

	void Mesh::ProcessNode(aiNode *node, const aiScene *scene, const char* filepath)
	{
		ARC_PROFILE_SCOPE();

		for(unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			const char* nodeName = node->mName.C_Str();
			ProcessMesh(mesh, scene, filepath, nodeName);
		}

		for(unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, filepath);
		}
	}

	eastl::vector<Ref<Texture2D>> LoadMaterialTextures(aiMaterial *mat, aiTextureType type, const char* filepath)
	{
		ARC_PROFILE_SCOPE();

		eastl::string path = eastl::string(filepath);
		eastl::string dir = path.substr(0, path.find_last_of('\\'));
		eastl::vector<Ref<Texture2D>> textures;
		for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			Ref<Texture2D> texture = AssetManager::GetTexture2D(dir + '\\' + str.C_Str());
			textures.push_back(texture);
		}
		return textures;
	}

	void Mesh::ProcessMesh(aiMesh *mesh, const aiScene *scene, const char* filepath, const char* nodeName)
	{
		ARC_PROFILE_SCOPE();

		eastl::vector<float> vertices;
		size_t count = mesh->mNumVertices * 14;
		vertices.reserve(count);

		eastl::vector<uint32_t> indices;
		for(size_t i = 0; i < mesh->mNumVertices; i++)
		{
			auto& vertexPosition = mesh->mVertices[i];
			vertices.push_back(vertexPosition.x);
			vertices.push_back(vertexPosition.y);
			vertices.push_back(vertexPosition.z);

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
        eastl::vector<Ref<Texture2D>> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, filepath);
		eastl::vector<Ref<Texture2D>> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, filepath);
		eastl::vector<Ref<Texture2D>> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, filepath);
		eastl::vector<Ref<Texture2D>> emissiveMaps = LoadMaterialTextures(material, aiTextureType_EMISSIVE, filepath);

		uint32_t index = m_Submeshes.size();
		m_Submeshes.emplace_back(nodeName, CreateRef<Material>(), vertexArray);
		Submesh& submesh = m_Submeshes[index];

		auto& materialProperties = submesh.Mat->GetShader()->GetMaterialProperties();
		bool normalMapApplied = false;
		for (auto& [name, property] : materialProperties)
		{
			if (property.Type == MaterialPropertyType::Sampler2D)
			{
				uint32_t slot = submesh.Mat->GetData<uint32_t>(name.c_str());

				if (diffuseMaps.size() > 0 && 
					(name.find("albedo") != eastl::string::npos || name.find("Albedo") != eastl::string::npos ||
					name.find("diff") != eastl::string::npos || name.find("Diff") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(slot, diffuseMaps[0]);
				}

				if (normalMaps.size() > 0 && 
					(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
					name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(slot, normalMaps[0]);
					normalMapApplied = true;
				}
				else if (heightMaps.size() > 0 && 
					(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
					name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(slot, heightMaps[0]);
					normalMapApplied = true;
				}

				if (emissiveMaps.size() > 0 && 	(name.find("emissi") != eastl::string::npos || name.find("Emissi") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(slot, emissiveMaps[0]);
				}
			}

			if (property.Type == MaterialPropertyType::Bool && normalMapApplied &&
				(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
				name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
			{
				submesh.Mat->SetData(name.c_str(), 1);
			}
		}
	}
}
