#pragma once

struct aiScene;
struct aiNode;
struct aiMesh;

namespace ArcEngine
{
	class VertexArray;
	class Material;

	struct Submesh
	{
		std::string Name;
		Ref<Material> Mat;
		Ref<VertexArray> Geometry;

		Submesh(const std::string& name, const Ref<Material>& material, const Ref<VertexArray>& geometry)
			: Name(name), Mat(material), Geometry(geometry)
		{
		}
	};

	class Mesh
	{
	public:
		Mesh() = default;
		explicit Mesh(const char* filepath);
		virtual ~Mesh() = default;

		void Load(const char* filepath);

		Submesh& GetSubmesh(size_t index);
		size_t GetSubmeshCount() const { return m_Submeshes.size(); }
		const char* GetName() const { return m_Name.c_str(); }
		const char* GetFilepath() const { return m_Filepath.c_str(); }

	private:
		std::string m_Name;
		std::string m_Filepath;
		std::vector<Submesh> m_Submeshes;
	};
}
