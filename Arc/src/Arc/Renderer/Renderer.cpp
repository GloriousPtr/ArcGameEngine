#include "arcpch.h"
#include "Arc/Renderer/Renderer.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"

#include "Shader.h"
#include "Buffer.h"
#include "RenderGraphData.h"
#include "VertexArray.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace ArcEngine
{
	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

	struct Data
	{
		Ref<VertexArray> va;
		Ref<Shader> shader;
		Ref<ConstantBuffer> transform;
		Ref<ConstantBuffer> cam;
		Ref<Texture2D> tex;

		glm::mat4 view;
		glm::mat4 projection;
		uint32_t width;
		uint32_t height;
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

		const BufferLayout layout
		{
			{ ShaderDataType::Float3, "POSITION" },
			{ ShaderDataType::Float2, "TEXCOORD" }
		};

		s_Data->shader = Shader::Create("assets/shaders/FlatColor.hlsl", layout);

		// Quad
		{
			constexpr float vertices[20] = {
				 -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, // top left
				  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // bottom right
				 -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom left
				  0.5f,  0.5f, 0.0f, 1.0f, 0.0f  // top right
			};
			constexpr uint32_t indices[6] = {
				0, 1, 2,
				0, 3, 1
			};
			s_Data->va = VertexArray::Create();
			Ref<VertexBuffer> quadVertexBuffer = VertexBuffer::Create(vertices, 20 * sizeof(float), 5 * sizeof(float));
			quadVertexBuffer->SetLayout(layout);
			s_Data->va->AddVertexBuffer(quadVertexBuffer);
			Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(indices, 6);
			s_Data->va->SetIndexBuffer(quadIndexBuffer);
		}

		s_Data->tex = Texture2D::Create("Resources/Textures/Texel Density Texture 1.png");

		s_Data->transform = ConstantBuffer::Create(sizeof(glm::mat4), 100, 5);
		s_Data->cam = ConstantBuffer::Create(sizeof(glm::mat4), 1, 4);

		s_Data->width = 1600;
		s_Data->height = 900;
		s_Data->view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		s_Data->projection = glm::perspective(glm::radians(45.0f), 1600.0f / 900.0f, 0.01f, 1000.0f);

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

		s_Data->width = width;
		s_Data->height = height;
		s_Data->projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.01f, 1000.0f);
	}

	void Renderer::OnRender()
	{
		static glm::vec3 cameraPosition = { 0.0f, 0.0f, 1.0f };
		static float fov = 45.0f;

		static glm::vec3 position1	= { 1.0f, 0.0f, 0.0f };
		static glm::vec3 rotation1	= { 0.0f, 0.0f, 0.0f };
		static glm::vec3 scale1		= { 1.0f, 1.0f, 1.0f };

		static glm::vec3 position2	= {-2.0f, 0.0f, 0.0f };
		static glm::vec3 rotation2	= { 0.0f, 0.0f, 0.0f };
		static glm::vec3 scale2		= { 1.0f, 1.0f, 1.0f };

		static glm::vec4 clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

		ImGui::Begin("Settings");
		{
			ImGui::ColorEdit3("Clear Color", glm::value_ptr(clearColor));

			ImGui::Separator();

			if (ImGui::DragFloat3("Camera Position", glm::value_ptr(cameraPosition), 0.1f))
				s_Data->view = glm::lookAt(cameraPosition, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

			if (ImGui::DragFloat("FOV", &fov, 0.1f, 30.0f, 120.0f))
				s_Data->projection = glm::perspective(glm::radians(fov), (float)s_Data->width / (float)s_Data->height, 0.01f, 1000.0f);

			ImGui::DragFloat3("Position 1", glm::value_ptr(position1), 0.1f);
			glm::vec3 degrees1 = glm::degrees(rotation1);
			if (ImGui::DragFloat3("Rotation 1", glm::value_ptr(degrees1), 0.1f))
				rotation1 = glm::radians(degrees1);
			ImGui::DragFloat3("Scale 1", glm::value_ptr(scale1), 0.1f);

			ImGui::Separator();

			ImGui::DragFloat3("Position 2", glm::value_ptr(position2), 0.1f);
			glm::vec3 degrees2 = glm::degrees(rotation2);
			if (ImGui::DragFloat3("Rotation 2", glm::value_ptr(degrees2), 0.1f))
				rotation2 = glm::radians(degrees2);
			ImGui::DragFloat3("Scale 2", glm::value_ptr(scale2), 0.1f);

		}
		ImGui::End();




		RenderCommand::SetClearColor(clearColor);
		s_Data->shader->Bind();

		glm::mat4 viewProj = s_Data->projection * s_Data->view;
		s_Data->cam->Bind(0);
		s_Data->cam->SetData(&viewProj, sizeof(viewProj), 0);

		s_Data->tex->Bind(2);

		glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), position1) * glm::toMat4(glm::quat(rotation1)) * glm::scale(glm::mat4(1.0f), scale1);
		s_Data->transform->Bind(0);
		s_Data->transform->SetData(&transform1, sizeof(glm::mat4), 0);
		RenderCommand::DrawIndexed(s_Data->va);

		glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), position2) * glm::toMat4(glm::quat(rotation2)) * glm::scale(glm::mat4(1.0f), scale2);
		s_Data->transform->Bind(1);
		s_Data->transform->SetData(&transform2, sizeof(glm::mat4), 1);
		RenderCommand::DrawIndexed(s_Data->va);
	}
}
