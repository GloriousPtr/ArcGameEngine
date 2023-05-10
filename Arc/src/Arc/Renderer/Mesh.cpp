#include "arcpch.h"
#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Arc/Core/AssetManager.h"
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

namespace eastl
{
	template<> struct hash<ArcEngine::Vertex>
	{
		size_t operator()(ArcEngine::Vertex const& vertex) const noexcept
		{
			const std::size_t h1 = std::hash<glm::vec3>{}(vertex.Position);
			const std::size_t h2 = std::hash<glm::vec2>{}(vertex.TexCoord);
			const std::size_t h3 = std::hash<glm::vec3>{}(vertex.Normal);
			const std::size_t h4 = std::hash<glm::vec3>{}(vertex.Tangent);
			const std::size_t h5 = std::hash<glm::vec3>{}(vertex.Bitangent);
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

		std::filesystem::path path = filepath;

		if (!std::filesystem::exists(path))
			return;

		auto ext = path.extension();
		bool supportedFile = ext == ".obj";
		if (!supportedFile)
		{
			ARC_CORE_ERROR("{} file(s) not supported: {}", ext, filepath);
			return;
		}

		if (ext == ".obj")
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
			for (const auto& shape : shapes)
			{
				eastl::vector<Vertex> vertices;
				eastl::vector<uint32_t> indices;
				eastl::hash_map<Vertex, uint32_t> uniqueVertices{};

				// Loop over faces(polygon)
				int materialId = -1;
				size_t index_offset = 0;
				for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
				{
					auto fv = static_cast<size_t>(shape.mesh.num_face_vertices[f]);

					// Loop over vertices in the face.
					for (size_t v = 0; v < fv; v++)
					{
						// access to vertex
						tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

						Vertex vertex;
						vertex.Position.x = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0];
						vertex.Position.y = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1];
						vertex.Position.z = -attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2];
						if (idx.texcoord_index >= 0)
						{
							vertex.TexCoord.x = attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 0];
							vertex.TexCoord.y = 1.0f - attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 1];
						}
						if (idx.normal_index >= 0)
						{
							vertex.Normal.x = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 0];
							vertex.Normal.y = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 1];
							vertex.Normal.z = -attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 2];

							int v0_index = shape.mesh.indices[index_offset + 0].vertex_index;
							int v1_index = shape.mesh.indices[index_offset + 1].vertex_index;
							int v2_index = shape.mesh.indices[index_offset + 2].vertex_index;

							glm::vec3 v0 = { attrib.vertices[3 * v0_index + 0], attrib.vertices[3 * v0_index + 1], -attrib.vertices[3 * v0_index + 2] };
							glm::vec3 v1 = { attrib.vertices[3 * v1_index + 0], attrib.vertices[3 * v1_index + 1], -attrib.vertices[3 * v1_index + 2] };
							glm::vec3 v2 = { attrib.vertices[3 * v2_index + 0], attrib.vertices[3 * v2_index + 1], -attrib.vertices[3 * v2_index + 2] };

							int uv0_index = shape.mesh.indices[index_offset + 0].texcoord_index;
							int uv1_index = shape.mesh.indices[index_offset + 1].texcoord_index;
							int uv2_index = shape.mesh.indices[index_offset + 2].texcoord_index;

							glm::vec2 uv0 = { attrib.texcoords[2 * uv0_index + 0], 1.0f - attrib.texcoords[2 * uv0_index + 1] };
							glm::vec2 uv1 = { attrib.texcoords[2 * uv1_index + 0], 1.0f - attrib.texcoords[2 * uv1_index + 1] };
							glm::vec2 uv2 = { attrib.texcoords[2 * uv2_index + 0], 1.0f - attrib.texcoords[2 * uv2_index + 1] };

							auto deltaPos1 = v1 - v0;
							auto deltaPos2 = v2 - v0;
							auto deltaUV1 = uv1 - uv0;
							auto deltaUV2 = uv2 - uv0;

							float det = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
							if (det == 0.0f)
							{
								vertex.Tangent = { 1.0f, 0.0f, 0.0f };
								vertex.Bitangent = { 0.0f, 1.0f, 0.0f };
							}
							else
							{
								float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
								vertex.Tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
								vertex.Bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
							}

							auto t = glm::normalize(vertex.Tangent - vertex.Normal * glm::dot(vertex.Normal, vertex.Tangent));
							auto b = glm::normalize(vertex.Bitangent - vertex.Normal * glm::dot(vertex.Normal, vertex.Bitangent));
							float handedness = glm::dot(glm::cross(vertex.Normal, t), b);
							if (handedness < 0.0f)
							{
								vertex.Tangent = -vertex.Tangent;
								vertex.Bitangent = -vertex.Bitangent;
							}
						}

						if (uniqueVertices.count(vertex) == 0)
						{
							uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
							vertices.push_back(vertex);
						}

						indices.push_back(uniqueVertices[vertex]);
					}
					index_offset += fv;

					// per-face material
					materialId = shape.mesh.material_ids[f];
				}

				Ref<VertexArray> vertexArray = VertexArray::Create();

				Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(reinterpret_cast<float*>(vertices.data()), static_cast<uint32_t>(sizeof(Vertex) * vertices.size()), sizeof(Vertex));
				vertexArray->AddVertexBuffer(vertexBuffer);

				Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
				vertexArray->SetIndexBuffer(indexBuffer);

				const Submesh& submesh = m_Submeshes.emplace_back(shape.name.c_str(), CreateRef<Material>(), vertexArray);

				if (materialId >= 0)
				{
					const auto& material = materials.at(materialId);
					
					const auto& materialProperties = submesh.Mat->GetProperties();
					bool normalMapApplied = false;

					std::filesystem::path dir = path.parent_path();

					for (const auto& property : materialProperties)
					{
						if (property.Type == MaterialPropertyType::Texture2D ||
							property.Type == MaterialPropertyType::Texture2DBindless)
						{
							if (!material.diffuse_texname.empty() &&
								(property.Name.find("albedo") != eastl::string::npos || property.Name.find("Albedo") != eastl::string::npos ||
									property.Name.find("diff") != eastl::string::npos || property.Name.find("Diff") != eastl::string::npos))
							{
								eastl::string pathStr = (dir / material.diffuse_texname).string().c_str();
								submesh.Mat->SetTexture(property.Name, AssetManager::GetTexture2D(pathStr));
							}

							if (!material.normal_texname.empty() &&
								(property.Name.find("norm") != eastl::string::npos || property.Name.find("Norm") != eastl::string::npos ||
									property.Name.find("height") != eastl::string::npos || property.Name.find("Height") != eastl::string::npos))
							{
								std::string pathStr = (dir / material.normal_texname).string();
								submesh.Mat->SetTexture(property.Name, AssetManager::GetTexture2D(pathStr.c_str()));
								normalMapApplied = true;
							}
							else if (!material.bump_texname.empty() &&
								(property.Name.find("norm") != eastl::string::npos || property.Name.find("Norm") != eastl::string::npos ||
									property.Name.find("height") != eastl::string::npos || property.Name.find("Height") != eastl::string::npos))
							{
								std::string pathStr = (dir / material.bump_texname).string();
								submesh.Mat->SetTexture(property.Name, AssetManager::GetTexture2D(pathStr.c_str()));
								normalMapApplied = true;
							}

							if (!material.emissive_texname.empty() &&
								(property.Name.find("emissi") != eastl::string::npos || property.Name.find("Emissi") != eastl::string::npos))
							{
								std::string pathStr = (dir / material.emissive_texname).string();
								submesh.Mat->SetTexture(property.Name, AssetManager::GetTexture2D(pathStr.c_str()));
							}
						}

						if (property.Type == MaterialPropertyType::Bool && normalMapApplied &&
							(property.Name.find("norm") != eastl::string::npos || property.Name.find("Norm") != eastl::string::npos ||
								property.Name.find("height") != eastl::string::npos || property.Name.find("Height") != eastl::string::npos))
						{
							submesh.Mat->SetData(property.Name, 1);
						}
					}
				}
			}
		}

		m_Filepath = filepath;
		m_Name = StringUtils::GetName(filepath);
	}

	Submesh& Mesh::GetSubmesh(size_t index)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(index < m_Submeshes.size(), "Submesh index out of bounds");

		return m_Submeshes[index];
	}
}
