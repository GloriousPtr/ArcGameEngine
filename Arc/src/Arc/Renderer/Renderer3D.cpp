#include "arcpch.h"
#include "Renderer3D.h"

#include "Renderer.h"
#include "Framebuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ArcEngine
{
	struct MeshData
	{
		glm::mat4 Transform = glm::mat4(1.0f);
		Ref<VertexArray> VertexArray = nullptr;
		
		MeshComponent::CullModeType CullMode;

		glm::vec4 AlbedoColor = glm::vec4(1.0f);
		float NormalStrength = 0.5f;
		float Metallic = 0.5f;
		float Roughness = 0.5f;
		float AO = 0.5f;
		glm::vec3 EmissiveColor = glm::vec3(0.0f);
		float EmissiveIntensity = 5.0f;

		bool UseAlbedoMap = false;
		bool UseNormalMap = false;
		bool UseMetallicMap = false;
		bool UseRoughnessMap = false;
		bool UseOcclusionMap = false;
		bool UseEmissiveMap = false;

		Ref<Texture2D> AlbedoMap = nullptr;
		Ref<Texture2D> NormalMap = nullptr;
		Ref<Texture2D> MetallicMap = nullptr;
		Ref<Texture2D> RoughnessMap = nullptr;
		Ref<Texture2D> AmbientOcclusionMap = nullptr;
		Ref<Texture2D> EmissiveMap = nullptr;
	};

	static std::vector<MeshData> meshes;
	static Ref<Shader> shader;
	static Ref<Shader> shadowMapShader;
	static Ref<Shader> cubemapShader;
	static Ref<VertexArray> cubeVertexArray;
	static Ref<UniformBuffer> ubCamera;
	static Ref<UniformBuffer> ubLights;

	static glm::mat4 cameraView;
	static glm::mat4 cameraProjection;
	static Entity skylight;
	static std::vector<Entity> sceneLights;
	glm::mat4 dirLightView;
	glm::mat4 dirLightViewProj;

	static Ref<Framebuffer> shadowMapFramebuffer;

	ShaderLibrary Renderer3D::s_ShaderLibrary;

	void Renderer3D::Init()
	{
		FramebufferSpecification spec;
		spec.Width = 4096;
		spec.Height = 4096;
		shadowMapFramebuffer = Framebuffer::Create(spec);

		ubCamera = UniformBuffer::Create();
		ubCamera->SetLayout({
			{ ShaderDataType::Mat4, "u_View" },
			{ ShaderDataType::Mat4, "u_Projection" },
			{ ShaderDataType::Mat4, "u_ViewProjection" },
			{ ShaderDataType::Float4, "u_CameraPosition" }
		}, 0);

		ubLights = UniformBuffer::Create();
		ubLights->SetLayout({
			{ ShaderDataType::Float4, "u_Position" },
			{ ShaderDataType::Float4, "u_Color" },
			{ ShaderDataType::Float4, "u_AttenFactors" },
			{ ShaderDataType::Float4, "u_LightDir" },
			{ ShaderDataType::Float4, "u_Intensity" },
		}, 1, 26);													// 25 is max number of lights

		s_ShaderLibrary = ShaderLibrary();
		shadowMapShader = s_ShaderLibrary.Load("assets/shaders/DepthShader.glsl");
		cubemapShader = s_ShaderLibrary.Load("assets/shaders/Cubemap.glsl");
		shader = s_ShaderLibrary.Load("assets/shaders/PBR.glsl");

		shader->Bind();
		shader->SetUniformBlock("Camera", 0);
		shader->SetUniformBlock("LightBuffer", 1);

		// Cubemap
		{
			float vertices[108] = {
				// back face
				 0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,

				-0.5f,  0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,
			
				 // front face
				 0.5f,  0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,
				-0.5f, -0.5f,  0.5f,

				-0.5f, -0.5f,  0.5f,
				-0.5f,  0.5f,  0.5f,
				 0.5f,  0.5f,  0.5f,
			
				 // left face
				-0.5f, -0.5f, -0.5f,
				-0.5f,  0.5f, -0.5f,
				-0.5f,  0.5f,  0.5f,
			
				-0.5f,  0.5f,  0.5f,
				-0.5f, -0.5f,  0.5f,
				-0.5f, -0.5f, -0.5f,
				// right face
				 0.5f,  0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f,  0.5f,

				 0.5f, -0.5f,  0.5f,
				 0.5f,  0.5f,  0.5f,
				 0.5f, -0.5f, -0.5f,
				// bottom face
				 0.5f, -0.5f,  0.5f,
				 0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,

				-0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,
				// top face
				 0.5f,  0.5f, -0.5f,
				 0.5f,  0.5f , 0.5f,
				-0.5f,  0.5f, -0.5f,

				-0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f, -0.5f,
				 0.5f,  0.5f,  0.5f,
			};

			Ref<VertexBuffer> cubeVertexBuffer = VertexBuffer::Create(vertices, 108 * sizeof(float));
			cubeVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" }
			});

			cubeVertexArray = VertexArray::Create();
			cubeVertexArray->AddVertexBuffer(cubeVertexBuffer);
		}
	}

	void Renderer3D::Shutdown()
	{
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		ARC_PROFILE_FUNCTION();

	}

	void Renderer3D::BeginScene(const EditorCamera& camera, Entity cubemap, std::vector<Entity>& lights)
	{
		ARC_PROFILE_FUNCTION();
		
		cameraView = camera.GetViewMatrix();
		cameraProjection = camera.GetProjection();

		skylight = cubemap;
		sceneLights = lights;

		SetupCameraData(camera);
		SetupLightsData();
	}

	void Renderer3D::EndScene(Ref<Framebuffer>& renderTarget)
	{
		Flush(renderTarget);

		shader->Unbind();
	}

	void Renderer3D::DrawCube()
	{
		RenderCommand::Draw(cubeVertexArray, 36);
	}

	void Renderer3D::SubmitMesh(uint32_t entityID, MeshComponent& meshComponent, const glm::mat4& transform)
	{
		ARC_PROFILE_FUNCTION();

		MeshData mesh;
		mesh.VertexArray = meshComponent.VertexArray;
		mesh.Transform = transform;

		mesh.CullMode = meshComponent.CullMode;

		mesh.AlbedoColor = meshComponent.AlbedoColor;
		mesh.NormalStrength = meshComponent.NormalStrength;
		mesh.Metallic = meshComponent.Metallic;
		mesh.Roughness = meshComponent.Roughness;
		mesh.AO = meshComponent.AO;
		mesh.EmissiveColor = meshComponent.EmissiveColor;
		mesh.EmissiveIntensity = meshComponent.EmissiveIntensity;

		mesh.UseAlbedoMap = meshComponent.UseAlbedoMap;
		mesh.UseNormalMap = meshComponent.UseNormalMap;
		mesh.UseMetallicMap = meshComponent.UseMetallicMap;
		mesh.UseRoughnessMap = meshComponent.UseRoughnessMap;
		mesh.UseOcclusionMap = meshComponent.UseOcclusionMap;
		mesh.UseEmissiveMap = meshComponent.UseEmissiveMap;

		mesh.AlbedoMap = meshComponent.AlbedoMap;
		mesh.NormalMap = meshComponent.NormalMap;
		mesh.MetallicMap = meshComponent.MetallicMap;
		mesh.RoughnessMap = meshComponent.RoughnessMap;
		mesh.AmbientOcclusionMap = meshComponent.AmbientOcclusionMap;
		mesh.UseEmissiveMap = meshComponent.UseEmissiveMap;
		
		meshes.push_back(mesh);
	}

	uint32_t Renderer3D::GetShadowMapTextureID()
	{
		return shadowMapFramebuffer->GetDepthAttachmentRendererID();
	}

	void Renderer3D::Flush(Ref<Framebuffer>& renderTarget)
	{
		ARC_PROFILE_FUNCTION();
		ShadowMapPass();
		RenderPass(renderTarget);
		meshes.clear();
	}

	void Renderer3D::SetupCameraData(const EditorCamera& camera)
	{
		ubCamera->Bind();
		ubCamera->SetData((void*)glm::value_ptr(camera.GetViewMatrix()), 0, sizeof(glm::mat4));
		ubCamera->SetData((void*)glm::value_ptr(camera.GetProjection()), sizeof(glm::mat4), sizeof(glm::mat4));
		ubCamera->SetData((void*)glm::value_ptr(camera.GetViewProjection()), 2 * sizeof(glm::mat4), sizeof(glm::mat4));
		ubCamera->SetData((void*)glm::value_ptr(camera.GetPosition()), 3 * sizeof(glm::mat4), sizeof(glm::vec4));
	}

	void Renderer3D::SetupLightsData()
	{
		uint32_t numLights = 0;
		uint32_t size = 5 * sizeof(glm::vec4);
		ubLights->Bind();
		for (Entity e : sceneLights)
		{
			TransformComponent transformComponent = e.GetComponent<TransformComponent>();
			LightComponent lightComponent = e.GetComponent<LightComponent>();
			glm::mat4& worldTransform = e.GetWorldTransform();
			glm::vec4 attenFactors = glm::vec4(
				lightComponent.Constant,
				lightComponent.Linear,
				lightComponent.Quadratic,
				static_cast<uint32_t>(lightComponent.Type));

			uint32_t offset = 0;
			ubLights->SetData((void*)glm::value_ptr(worldTransform[3]), size * numLights + offset, sizeof(glm::vec4));

			offset += sizeof(glm::vec4);
			ubLights->SetData((void*)glm::value_ptr(lightComponent.Color), size * numLights + offset, sizeof(glm::vec4));

			offset += sizeof(glm::vec4);
			ubLights->SetData((void*)glm::value_ptr(attenFactors), size * numLights + offset, sizeof(glm::vec4));

			offset += sizeof(glm::vec4);
			// Based off of +Z direction
			glm::vec4 zDir = worldTransform * glm::vec4(0, 0, 1, 0);
			ubLights->SetData((void*)glm::value_ptr(zDir), size * numLights + offset, sizeof(glm::vec4));

			offset += sizeof(glm::vec4);
			ubLights->SetData(&lightComponent.Intensity, size * numLights + offset, sizeof(float));

			numLights++;
		}

		// Pass number of lights within the scene
		// 25 is max number of lights
		ubLights->SetData(&numLights, 25 * size, sizeof(uint32_t));
	}

	void Renderer3D::RenderPass(Ref<Framebuffer>& renderTarget)
	{
		renderTarget->Bind();

		float skylightIntensity = 0.0f;
		SkyLightComponent* skylightComponent = nullptr;
		if (skylight)
		{
			skylightComponent = &(skylight.GetComponent<SkyLightComponent>());
			if (skylightComponent->Texture)
			{
				RenderCommand::SetDepthMask(false);

				skylightIntensity = skylightComponent->Intensity;
				skylightComponent->Texture->Bind(0);
				cubemapShader->Bind();
				cubemapShader->SetMat4("u_View", cameraView);
				cubemapShader->SetMat4("u_Projection", cameraProjection);
				DrawCube();
				cubemapShader->Unbind();

				RenderCommand::SetDepthMask(true);

				skylightComponent->Texture->Bind(1);
			}
		}

		MeshComponent::CullModeType currentCullMode = MeshComponent::CullModeType::Unknown;
		for (auto it = meshes.rbegin(); it != meshes.rend(); it++)
		{
			MeshData* meshData = &(*it);

			shader->Bind();

			shader->SetFloat("u_IrradianceIntensity", skylightIntensity);
			
			shader->SetFloat4("u_Albedo", meshData->AlbedoColor);
			shader->SetFloat("u_NormalStrength", meshData->NormalStrength);
			shader->SetFloat("u_Metallic", meshData->Metallic);
			shader->SetFloat("u_Roughness", meshData->Roughness);
			shader->SetFloat("u_AO", meshData->AO);
			shader->SetFloat3("u_EmissionColor", meshData->EmissiveColor);
			shader->SetFloat("u_EmissiveIntensity", meshData->EmissiveIntensity);

			shader->SetInt("u_UseAlbedoMap", static_cast<int>(meshData->UseAlbedoMap));
			shader->SetInt("u_UseNormalMap", static_cast<int>(meshData->UseNormalMap));
			shader->SetInt("u_UseMetallicMap", static_cast<int>(meshData->UseMetallicMap));
			shader->SetInt("u_UseRoughnessMap", static_cast<int>(meshData->UseRoughnessMap));
			shader->SetInt("u_UseOcclusionMap", static_cast<int>(meshData->UseOcclusionMap));
			shader->SetInt("u_UseEmissiveMap", static_cast<int>(meshData->UseEmissiveMap));

			shader->SetInt("u_IrradianceMap", 0);
			shader->SetInt("u_DirectionalShadowMap", 1);

			shader->SetInt("u_AlbedoMap", 2);
			shader->SetInt("u_NormalMap", 3);
			shader->SetInt("u_MetallicMap", 4);
			shader->SetInt("u_RoughnessMap", 5);
			shader->SetInt("u_AmbientOcclusionMap", 6);
			shader->SetInt("u_EmissiveMap", 7);

			shadowMapFramebuffer->BindDepthAttachment(1);

			if (meshData->AlbedoMap)
				meshData->AlbedoMap->Bind(2);
			if (meshData->NormalMap)
				meshData->NormalMap->Bind(3);
			if (meshData->MetallicMap)
				meshData->MetallicMap->Bind(4);
			if (meshData->RoughnessMap)
				meshData->RoughnessMap->Bind(5);
			if (meshData->AmbientOcclusionMap)
				meshData->AmbientOcclusionMap->Bind(6);
			if (meshData->EmissiveMap)
				meshData->EmissiveMap->Bind(7);
			
			shader->SetMat4("u_Model", meshData->Transform);
			shader->SetMat4("u_DirLightView", dirLightView);
			shader->SetMat4("u_DirLightViewProj", dirLightViewProj);

			if (currentCullMode != meshData->CullMode)
			{
				currentCullMode = meshData->CullMode;
				switch (currentCullMode)
				{
				case MeshComponent::CullModeType::DoubleSided:
					RenderCommand::DisableCulling();
					break;
				case MeshComponent::CullModeType::Front:
					RenderCommand::EnableCulling();
					RenderCommand::FrontCull();
					break;
				case MeshComponent::CullModeType::Back:
					RenderCommand::EnableCulling();
					RenderCommand::BackCull();
					break;
				default:
					ARC_CORE_ASSERT(false);
				}
			}

			RenderCommand::DrawIndexed(meshData->VertexArray);
		}

		renderTarget->Unbind();
	}

	void Renderer3D::ShadowMapPass()
	{
//		RenderCommand::FrontCull();
		for (size_t i = 0; i < sceneLights.size(); i++)
		{
			Entity e = sceneLights[i];
			LightComponent light = e.GetComponent<LightComponent>();
			if (light.Type == LightComponent::LightType::Point)
				continue;

			shadowMapFramebuffer->Bind();
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
			RenderCommand::Clear();

			glm::mat4 transform = e.GetWorldTransform();
			float near_plane = -100.0f, far_plane = 100.0f;
			
			glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
			
			glm::mat4 inverted = glm::inverse(transform);
			glm::vec3 pos = transform[3];
			glm::vec3 dir = glm::normalize(glm::vec3(inverted[2]));
			dir.z *= -1;
			glm::vec3 lookAt = pos - dir;
			dirLightView = glm::lookAt(pos, lookAt, glm::vec3(0, 1 ,0));

			dirLightViewProj = lightProjection * dirLightView;

			shadowMapShader->Bind();
			shadowMapShader->SetMat4("u_ViewProjection", dirLightViewProj);

			for (auto it = meshes.rbegin(); it != meshes.rend(); it++)
			{
				MeshData* meshData = &(*it);

				shadowMapShader->SetMat4("u_Model", meshData->Transform);
				RenderCommand::DrawIndexed(meshData->VertexArray);
			}

			shadowMapFramebuffer->Unbind();
			
			break;
		}
//		RenderCommand::BackCull();
	}
}
