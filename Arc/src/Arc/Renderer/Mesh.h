#pragma once

#include "Arc/Core/Base.h"
#include "VertexArray.h"
#include "Material.h"

struct aiScene;
struct aiNode;
struct aiMesh;

namespace ArcEngine
{
	struct Submesh
	{
		eastl::string Name;
		Ref<Material> Mat;
		Ref<VertexArray> Geometry;

		Submesh(const char* name, Ref<Material>& material, Ref<VertexArray>& geometry)
			: Name(name), Mat(material), Geometry(geometry)
		{
		}
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const char* filepath);
		virtual ~Mesh() = default;

		void Load(const char* filepath);

		Submesh& GetSubmesh(uint32_t index);
		const uint32_t GetSubmeshCount() const { return m_Submeshes.size(); }
		const char* GetName() const { return m_Name.c_str(); }

	private:
		void ProcessNode(aiNode *node, const aiScene *scene, const char* filepath);
		void ProcessMesh(aiMesh *mesh, const aiScene *scene, const char* filepath, const char* nodeName);

	private:
		eastl::string m_Name;
		eastl::vector<Submesh> m_Submeshes;
	};
}
