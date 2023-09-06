#pragma once

namespace ArcEngine
{
	class VertexArray;
	class Material;

	struct Submesh
	{
		eastl::string Name;
		Ref<Material> Mat;
		Ref<VertexArray> Geometry;

		Submesh(const eastl::string& name, const Ref<Material>& material, const Ref<VertexArray>& geometry)
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

		[[nodiscard]] Submesh& GetSubmesh(size_t index);
		[[nodiscard]] size_t GetSubmeshCount() const { return m_Submeshes.size(); }
		[[nodiscard]] const char* GetName() const { return m_Name.c_str(); }
		[[nodiscard]] const char* GetFilepath() const { return m_Filepath.c_str(); }

	private:
		eastl::string m_Name;
		eastl::string m_Filepath;
		eastl::vector<Submesh> m_Submeshes;
	};
}
