#include "arcpch.h"
#include "Renderer3D.h"
#include "Renderer.h"

#include <glm/glm.hpp>

namespace ArcEngine
{
	static Ref<Shader> shader;

	void Renderer3D::Init()
	{
		shader = Shader::Create("assets/shaders/PBR.glsl");
	}

	void Renderer3D::Shutdown()
	{
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		shader->Bind();
		shader->SetMat4("u_ViewProjection", camera.GetViewProjection());
	}

	void Renderer3D::EndScene()
	{
		shader->Unbind();
	}

	void Renderer3D::DrawMesh(uint32_t entityID, Ref<VertexArray> vertexArray, Ref<Texture2D> texture, const glm::mat4& transform)
	{
		if (texture)
			texture->Bind();

		shader->SetMat4("u_Model", transform);
		vertexArray->Bind();
		vertexArray->GetIndexBuffer()->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}
