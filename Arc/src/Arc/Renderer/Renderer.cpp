#include "arcpch.h"
#include "Arc/Renderer/Renderer.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"

#include "Shader.h"
#include "Buffer.h"
#include "RenderGraphData.h"
#include "VertexArray.h"

namespace ArcEngine
{
	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

	struct Data
	{
		Ref<VertexArray> va;
		Ref<Shader> shader;
		Ref<ConstantBuffer> transform;
		Ref<ConstantBuffer> cam;

		glm::mat4 view;
		glm::mat4 projection;
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

		s_Data->transform = ConstantBuffer::Create(sizeof(glm::mat4), 2, 2);
		s_Data->cam = ConstantBuffer::Create(sizeof(glm::mat4), 1, 1);

		s_Data->view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), { 0.0f, 3.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		s_Data->projection = glm::perspective(90.0f, 1600.0f / 900.0f, 0.01f, 1000.0f);

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
		s_Data->projection = glm::perspective(90.0f, (float)width / (float)height, 0.01f, 1000.0f);
	}

	void Renderer::OnRender()
	{
		glm::vec3 position	= { 0.5f, 0.0f, 0.0f };
		glm::vec3 rotation	= { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale		= { 0.5f, 1.0f, 1.0f };

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		s_Data->shader->Bind();

		s_Data->view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		glm::mat4 viewProj = s_Data->projection * s_Data->view;
		s_Data->cam->Bind(0);
		s_Data->cam->SetData(&viewProj, sizeof(viewProj), 0);

		glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(glm::quat(rotation)) * glm::scale(glm::mat4(1.0f), scale);
		s_Data->transform->Bind(0);
		s_Data->transform->SetData(&transform1, sizeof(glm::mat4), 0);
		RenderCommand::DrawIndexed(s_Data->va);

		glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), -position) * glm::toMat4(glm::quat(rotation)) * glm::scale(glm::mat4(1.0f), scale);
		s_Data->transform->Bind(sizeof(glm::mat4));
		s_Data->transform->SetData(&transform2, sizeof(glm::mat4), sizeof(glm::mat4));
		RenderCommand::DrawIndexed(s_Data->va);
	}
}
