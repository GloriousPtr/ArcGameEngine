#include "arcpch.h"
#include "Arc/Renderer/Renderer.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"

namespace ArcEngine
{
	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

	struct Data
	{
		Ref<VertexArray> va;
		Ref<Shader> shader;
	};

	inline static Scope<Data> s_Data;

	void Renderer::Init()
	{
		ARC_PROFILE_SCOPE()

		s_Data = CreateScope<Data>();

		RenderCommand::Init();

		/*
		shader = Shader::Create("assets/shaders/TestShader.hlsl",
		{
			{ ShaderDataType::Float3, "POSITION"  },
			{ ShaderDataType::Float3, "NORMAL"    },
			{ ShaderDataType::Float3, "TANGENT"   },
			{ ShaderDataType::Float3, "BINORMAL"  },
			{ ShaderDataType::Float2, "TEXCOORD"  }
		});
		*/

		s_Data->shader = Shader::Create("assets/shaders/FlatColor.hlsl",
		{
			{ ShaderDataType::Float3, "POSITION" },
			{ ShaderDataType::Float4, "COLOR"    },
		});

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
		};

		Vertex vertices[] =
		{
			{ glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ glm::vec3(0.0f, 0.5f, 0.0f),  glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ glm::vec3(0.5f, 0.0f, 0.0f),  glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) }
		};

		uint32_t indices[] = { 0, 1, 2 };

		Ref<VertexBuffer> vb = VertexBuffer::Create((float*)(&vertices[0]), sizeof(vertices), sizeof(Vertex));
		Ref<IndexBuffer> ib = IndexBuffer::Create(indices, 3);
		s_Data->va = VertexArray::Create();
		s_Data->va->AddVertexBuffer(vb);
		s_Data->va->SetIndexBuffer(ib);

		//Renderer2D::Init();
		//Renderer3D::Init();
	}

	void Renderer::Shutdown()
	{
		ARC_PROFILE_SCOPE()

		s_Data = nullptr;

		//Renderer2D::Shutdown();
		//Renderer3D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE()

		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::OnRender()
	{
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		s_Data->shader->Bind();
		RenderCommand::DrawIndexed(s_Data->va);
	}
}
