#pragma once

#include "Arc/Core/Base.h"
#include "VertexArray.h"
#include "Material.h"

namespace ArcEngine
{
	struct Submesh
	{
		Ref<Material> Mat;
		Ref<VertexArray> Geometry;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const char* filepath);
		virtual ~Mesh() = default;

		const Submesh& GetSubmesh(uint32_t index) const;

	private:
		void LoadMesh(const char* filepath);
		void ProcessNode(aiNode *node, const aiScene *scene, const char* filepath);
		void ProcessMesh(aiMesh *mesh, const aiScene *scene, const char* filepath);

	private:
		std::vector<Submesh> m_Submeshes;
	};
}
