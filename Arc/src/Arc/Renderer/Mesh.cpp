#include "arcpch.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Arc/Core/AssetManager.h"
#include "Arc/Utils/StringUtils.h"

namespace ArcEngine
{
	struct Vertex
	{
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec2 TexCoord = glm::vec2(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f);
		glm::vec3 Tangent = glm::vec3(0.0f);
		glm::vec3 Bitangent = glm::vec3(0.0f);

		bool operator==(const Vertex& other) const
		{
			return	Position == other.Position &&
				TexCoord == other.TexCoord &&
				Normal == other.Normal &&
				Tangent == other.Tangent &&
				Bitangent == other.Bitangent;
		}
	};
}

namespace std
{
	template<> struct hash<ArcEngine::Vertex>
	{
		size_t operator()(ArcEngine::Vertex const& vertex) const
		{
			std::size_t h1 = std::hash<glm::vec3>{}(vertex.Position);
			std::size_t h2 = std::hash<glm::vec2>{}(vertex.TexCoord);
			std::size_t h3 = std::hash<glm::vec3>{}(vertex.Normal);
			std::size_t h4 = std::hash<glm::vec3>{}(vertex.Tangent);
			std::size_t h5 = std::hash<glm::vec3>{}(vertex.Bitangent);
			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
		}
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

		eastl::string ext = StringUtils::GetExtension(filepath);
		if (ext == "obj")
		{
			tinyobj::ObjReaderConfig reader_config;
			tinyobj::ObjReader reader;

			if (!reader.ParseFromFile(filepath, reader_config))
			{
				if (!reader.Error().empty())
				{
					ARC_CORE_ERROR("Could not import the file: {0}. Error: {1}", filepath, reader.Error());
					return;
				}
			}

			if (!reader.Warning().empty())
				ARC_CORE_WARN("File: {0}. Warning: {1}", filepath, reader.Warning());

			auto& attrib = reader.GetAttrib();
			auto& shapes = reader.GetShapes();
			auto& materials = reader.GetMaterials();
			
			// Loop over shapes
			for (size_t s = 0; s < shapes.size(); s++)
			{
				eastl::vector<Vertex> vertices;
				eastl::vector<uint32_t> indices;
				std::unordered_map<Vertex, uint32_t> uniqueVertices{};

				// Loop over faces(polygon)
				int materialId = -1;
				size_t index_offset = 0;
				for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
				{
					size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

					// Loop over vertices in the face.
					for (size_t v = 0; v < fv; v++)
					{
						// access to vertex
						tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

						Vertex vertex;
						vertex.Position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
						vertex.Position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
						vertex.Position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
						if (idx.texcoord_index >= 0)
						{
							vertex.TexCoord.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
							vertex.TexCoord.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
						}
						if (idx.normal_index >= 0)
						{
							vertex.Normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
							vertex.Normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
							vertex.Normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
						}

						/*
						if (idx.normal_index >= 0)
						{
							// Shortcuts for vertices
							glm::vec3 v0 = { attrib.vertices[3 * size_t(idx.vertex_index + 0) + 0], attrib.vertices[3 * size_t(idx.vertex_index + 0) + 1], attrib.vertices[3 * size_t(idx.vertex_index + 0) + 2] };
							glm::vec3 v1 = { attrib.vertices[3 * size_t(idx.vertex_index + 1) + 0], attrib.vertices[3 * size_t(idx.vertex_index + 1) + 1], attrib.vertices[3 * size_t(idx.vertex_index + 1) + 2] };
							glm::vec3 v2 = { attrib.vertices[3 * size_t(idx.vertex_index + 2) + 0], attrib.vertices[3 * size_t(idx.vertex_index + 2) + 1], attrib.vertices[3 * size_t(idx.vertex_index + 2) + 2] };

							glm::vec2 uv0(0.0f);
							glm::vec2 uv1(0.0f);
							glm::vec2 uv2(0.0f);
							// Shortcuts for UVs
							if (idx.texcoord_index >= 0)
							{
								uv0 = { attrib.texcoords[2 * size_t(idx.texcoord_index + 0) + 0], attrib.texcoords[2 * size_t(idx.texcoord_index + 0) + 1] };
								uv1 = { attrib.texcoords[2 * size_t(idx.texcoord_index + 1) + 0], attrib.texcoords[2 * size_t(idx.texcoord_index + 1) + 1] };
								uv2 = { attrib.texcoords[2 * size_t(idx.texcoord_index + 2) + 0], attrib.texcoords[2 * size_t(idx.texcoord_index + 2) + 1] };
							}

							// Edges of the triangle : position delta
							const auto deltaPos1 = v1 - v0;
							const auto deltaPos2 = v2 - v0;

							// UV delta
							const auto deltaUV1 = uv1 - uv0;
							const auto deltaUV2 = uv2 - uv0;

							const float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
							const auto tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
							const auto bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

							vertices.emplace_back(tangent.x);
							vertices.emplace_back(tangent.y);
							vertices.emplace_back(tangent.z);
							vertices.emplace_back(bitangent.x);
							vertices.emplace_back(bitangent.y);
							vertices.emplace_back(bitangent.z);
						}
						*/

						if (uniqueVertices.count(vertex) == 0)
						{
							uniqueVertices[vertex] = (uint32_t)(vertices.size());
							vertices.push_back(vertex);
						}

						indices.push_back(uniqueVertices[vertex]);

						// Optional: vertex colors
						// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
						// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
						// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
					}
					index_offset += fv;

					// per-face material
					materialId = shapes[s].mesh.material_ids[f];
				}

				Ref<VertexArray> vertexArray = VertexArray::Create();

				Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create((float*)vertices.data(), (sizeof(Vertex) * vertices.size()));
				vertexBuffer->SetLayout({
					{ ShaderDataType::Float3, "a_Position" },
					{ ShaderDataType::Float2, "a_TexCoord" },
					{ ShaderDataType::Float3, "a_Normal" },
					{ ShaderDataType::Float3, "a_Tangent" },
					{ ShaderDataType::Float3, "a_Bitangent" },
				});
				vertexArray->AddVertexBuffer(vertexBuffer);

				Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
				vertexArray->SetIndexBuffer(indexBuffer);

				const Submesh& submesh = m_Submeshes.emplace_back(shapes[s].name.c_str(), CreateRef<Material>(), vertexArray);

				if (materialId >= 0)
				{
					const auto& material = materials.at(materialId);
					
					const auto& materialProperties = submesh.Mat->GetShader()->GetMaterialProperties();
					bool normalMapApplied = false;

					eastl::string path = eastl::string(filepath);
					eastl::string dir = path.substr(0, path.find_last_of('\\'));

					for (const auto& [name, property] : materialProperties)
					{
						if (property.Type == MaterialPropertyType::Sampler2D)
						{
							uint32_t slot = submesh.Mat->GetData<uint32_t>(name.c_str());

							if (!material.diffuse_texname.empty() &&
								(name.find("albedo") != eastl::string::npos || name.find("Albedo") != eastl::string::npos ||
									name.find("diff") != eastl::string::npos || name.find("Diff") != eastl::string::npos))
							{
								submesh.Mat->SetTexture(slot, AssetManager::GetTexture2D(dir + '\\' + material.diffuse_texname.c_str()));
							}

							if (!material.normal_texname.empty() &&
								(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
									name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
							{
								submesh.Mat->SetTexture(slot, AssetManager::GetTexture2D(dir + '\\' + material.normal_texname.c_str()));
								normalMapApplied = true;
							}

							if (!material.emissive_texname.empty() &&
								(name.find("emissi") != eastl::string::npos || name.find("Emissi") != eastl::string::npos))
							{
								submesh.Mat->SetTexture(slot, AssetManager::GetTexture2D(dir + '\\' + material.emissive_texname.c_str()));
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
		}
		else
		{
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

			const aiScene* scene = importer.ReadFile(filepath, meshImportFlags);

			if (!scene)
			{
				ARC_CORE_ERROR("Could not import the file: {0}. Error: {1}", filepath, importer.GetErrorString());
				return;
			}

			ProcessNode(scene->mRootNode, scene, filepath);
			m_Name = StringUtils::GetName(filepath);
		}
	}

	Submesh& Mesh::GetSubmesh(size_t index)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(index < m_Submeshes.size(), "Submesh index out of bounds");

		return m_Submeshes[index];
	}

	void Mesh::ProcessNode(const aiNode *node, const aiScene *scene, const char* filepath)
	{
		ARC_PROFILE_SCOPE();

		for(unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			const char* nodeName = node->mName.C_Str();
			ProcessMesh(mesh, scene, filepath, nodeName);
		}

		for(unsigned int i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene, filepath);
	}

	eastl::vector<Ref<Texture2D>> LoadMaterialTextures(const aiMaterial *mat, aiTextureType type, const char* filepath)
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
			textures.emplace_back(texture);
		}
		return textures;
	}

	void Mesh::ProcessMesh(const aiMesh *mesh, const aiScene *scene, const char* filepath, const char* nodeName)
	{
		ARC_PROFILE_SCOPE();

		eastl::vector<float> vertices;
		size_t count = mesh->mNumVertices * 14;
		vertices.reserve(count);

		eastl::vector<uint32_t> indices;
		for(size_t i = 0; i < mesh->mNumVertices; i++)
		{
			const auto& vertexPosition = mesh->mVertices[i];
			vertices.emplace_back(vertexPosition.x);
			vertices.emplace_back(vertexPosition.y);
			vertices.emplace_back(vertexPosition.z);

			if(mesh->mTextureCoords[0])
            {
                vertices.emplace_back(mesh->mTextureCoords[0][i].x);
                vertices.emplace_back(mesh->mTextureCoords[0][i].y);
            }
            else
			{
				vertices.emplace_back(0.0f);
				vertices.emplace_back(0.0f);
			}
			
			const auto& normal = mesh->mNormals[i];
			vertices.emplace_back(normal.x);
			vertices.emplace_back(normal.y);
			vertices.emplace_back(normal.z);

			if (mesh->mTangents)
			{
				const auto& tangent = mesh->mTangents[i];
				vertices.emplace_back(tangent.x);
				vertices.emplace_back(tangent.y);
				vertices.emplace_back(tangent.z);

				const auto& bitangent = mesh->mBitangents[i];
				vertices.emplace_back(bitangent.x);
				vertices.emplace_back(bitangent.y);
				vertices.emplace_back(bitangent.z);
			}
			else
			{
				size_t index = i / 3;
				// Shortcuts for vertices
				const auto& v0 = mesh->mVertices[index + 0];
				const auto& v1 = mesh->mVertices[index + 1];
				const auto& v2 = mesh->mVertices[index + 2];

				glm::vec2 uv0(0.0f);
				glm::vec2 uv1(0.0f);
				glm::vec2 uv2(0.0f);
				// Shortcuts for UVs
				if (const auto uv = mesh->mTextureCoords[0])
				{
					uv0 = { uv[index + 0].x, uv[index + 0].x };
					uv1 = { uv[index + 1].x, uv[index + 1].y };
					uv2 = { uv[index + 2].x, uv[index + 2].y };
				}

				// Edges of the triangle : position delta
				const auto deltaPos1 = v1 - v0;
				const auto deltaPos2 = v2 - v0;

				// UV delta
				const auto deltaUV1 = uv1 - uv0;
				const auto deltaUV2 = uv2 - uv0;

				const float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				const auto tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
				const auto bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

				vertices.emplace_back(tangent.x);
				vertices.emplace_back(tangent.y);
				vertices.emplace_back(tangent.z);
				vertices.emplace_back(bitangent.x);
				vertices.emplace_back(bitangent.y);
				vertices.emplace_back(bitangent.z);
			}
		}

		for(size_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			indices.reserve(face.mNumIndices);
			for(size_t j = 0; j < face.mNumIndices; j++)
				indices.emplace_back(face.mIndices[j]);
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

        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        eastl::vector<Ref<Texture2D>> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, filepath);
		eastl::vector<Ref<Texture2D>> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, filepath);
		eastl::vector<Ref<Texture2D>> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, filepath);
		eastl::vector<Ref<Texture2D>> emissiveMaps = LoadMaterialTextures(material, aiTextureType_EMISSIVE, filepath);

		size_t index = m_Submeshes.size();
		m_Submeshes.emplace_back(nodeName, CreateRef<Material>(), vertexArray);
		const Submesh& submesh = m_Submeshes[index];

		const auto& materialProperties = submesh.Mat->GetShader()->GetMaterialProperties();
		bool normalMapApplied = false;
		for (const auto& [name, property] : materialProperties)
		{
			if (property.Type == MaterialPropertyType::Sampler2D)
			{
				uint32_t slot = submesh.Mat->GetData<uint32_t>(name.c_str());

				if (!diffuseMaps.empty() && 
					(name.find("albedo") != eastl::string::npos || name.find("Albedo") != eastl::string::npos ||
					name.find("diff") != eastl::string::npos || name.find("Diff") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(slot, diffuseMaps[0]);
				}

				if (!normalMaps.empty() &&
					(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
					name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(slot, normalMaps[0]);
					normalMapApplied = true;
				}
				else if (!heightMaps.empty() &&
					(name.find("norm") != eastl::string::npos || name.find("Norm") != eastl::string::npos ||
					name.find("height") != eastl::string::npos || name.find("Height") != eastl::string::npos))
				{
					submesh.Mat->SetTexture(slot, heightMaps[0]);
					normalMapApplied = true;
				}

				if (!emissiveMaps.empty() &&
					(name.find("emissi") != eastl::string::npos || name.find("Emissi") != eastl::string::npos))
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
