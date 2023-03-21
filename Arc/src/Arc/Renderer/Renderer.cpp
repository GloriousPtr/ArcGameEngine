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

#include "Material.h"

namespace ArcEngine
{
	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;
	Scope<PipelineLibrary> Renderer::s_PipelineLibrary;

	struct Data
	{
		Ref<Framebuffer> fb;

		Ref<PipelineState> pipeline;
		Ref<ConstantBuffer> cam;
		Ref<ConstantBuffer> transform;
		Ref<Mesh> mesh;

		glm::mat4 view;
		glm::mat4 projection;
		uint32_t width;
		uint32_t height;
		bool fbResizing = false;
	};

	inline static Scope<Data> s_Data;

	void Renderer::Init()
	{
		ARC_PROFILE_SCOPE()

		RenderCommand::Init();

		s_PipelineLibrary = CreateScope<PipelineLibrary>();

		/*
		s_Data = CreateScope<Data>();


		FramebufferSpecification spec{};
		spec.Width = 1600;
		spec.Height = 900;
		spec.Attachments = { FramebufferTextureFormat::RGBA32F, FramebufferTextureFormat::Depth };
		s_Data->fb = Framebuffer::Create(spec);

		s_Data->pipeline = s_PipelineLibrary->Load("assets/shaders/FlatColor.hlsl", { FramebufferTextureFormat::RGBA32F, FramebufferTextureFormat::Depth });
		s_Data->mesh = CreateRef<Mesh>("../Sandbox/Assets/Models/sponza/sponza.obj");

		s_Data->cam = ConstantBuffer::Create(sizeof(glm::mat4), 1, 0);
		s_Data->transform = ConstantBuffer::Create(sizeof(glm::mat4), 100, 1);

		s_Data->width = spec.Width;
		s_Data->height = spec.Height;
		s_Data->view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		s_Data->projection = glm::perspective(glm::radians(45.0f), (float)spec.Width / (float)spec.Height, 0.01f, 1000.0f);
		*/

		Renderer2D::Init();
		//Renderer3D::Init();
	}

	void Renderer::Shutdown()
	{
		ARC_PROFILE_SCOPE()

		s_PipelineLibrary = nullptr;
		delete s_SceneData;

		s_Data = nullptr;

		Renderer2D::Shutdown();
		//Renderer3D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		ARC_PROFILE_SCOPE()

		RenderCommand::SetViewport(0, 0, width, height);

		//s_Data->width = width;
		//s_Data->height = height;
	}

	void Renderer::OnRender()
	{
		/*
		static glm::vec3 cameraPosition = { 0.0f, 0.0f, 1.0f };
		static float fov = 45.0f;

		static glm::vec3 position1 = { 1.0f, 0.0f, 0.0f };
		static glm::vec3 rotation1 = { 0.0f, 0.0f, 0.0f };
		static glm::vec3 scale1 = { 1.0f, 1.0f, 1.0f };

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
		}
		ImGui::End();


		s_Data->fb->Bind();
		{
			RenderCommand::SetClearColor(clearColor);
			if (s_Data->pipeline && s_Data->pipeline->Bind())
			{
				glm::mat4 viewProj = s_Data->projection * s_Data->view;
				s_Data->cam->Bind(0);
				s_Data->cam->SetData(&viewProj, sizeof(viewProj), 0);

				glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), position1) * glm::toMat4(glm::quat(rotation1)) * glm::scale(glm::mat4(1.0f), scale1);

				s_Data->transform->Bind(0);
				s_Data->transform->SetData(&transform1, sizeof(glm::mat4), 0);
				const size_t submeshCount = s_Data->mesh->GetSubmeshCount();
				for (size_t i = 0; i < submeshCount; ++i)
				{
					const auto& submesh = s_Data->mesh->GetSubmesh(i);
					submesh.Mat->Bind();
					RenderCommand::DrawIndexed(submesh.Geometry);
				}
			}
		}
		s_Data->fb->Unbind();

		static float viewportWidth = 1600;
		static float viewportHeight = 900;

		s_Data->projection = glm::perspective(glm::radians(fov), viewportWidth / viewportHeight, 0.01f, 1000.0f);

		ImGui::Begin("Viewport");
		ImGui::Image((ImTextureID) s_Data->fb->GetColorAttachmentRendererID(0), { viewportWidth, viewportHeight });
		ImVec2 size = ImGui::GetWindowSize();
		viewportWidth = size.x;
		viewportHeight = size.y;
		ImGui::End();

		const auto& specs = s_Data->fb->GetSpecification();
		bool shouldResize = specs.Width != static_cast<uint32_t>(viewportWidth) || specs.Height != static_cast<uint32_t>(viewportHeight);
		if (!s_Data->fbResizing && shouldResize)
		{
			s_Data->fbResizing = true;
			s_Data->fb->Resize(viewportWidth, viewportHeight);
		}

		if (s_Data->fbResizing && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			s_Data->fbResizing = false;
			*/
	}
}
