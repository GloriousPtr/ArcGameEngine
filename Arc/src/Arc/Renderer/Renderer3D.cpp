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
		bool UseMRAMap = false;
		bool UseEmissiveMap = false;

		Ref<Texture2D> AlbedoMap = nullptr;
		Ref<Texture2D> NormalMap = nullptr;
		Ref<Texture2D> MRAMap = nullptr;
		Ref<Texture2D> EmissiveMap = nullptr;
	};

	static std::vector<MeshData> meshes;
	static Ref<Shader> shader;
	static Ref<Shader> shadowMapShader;
	static Ref<Shader> cubemapShader;
	static Ref<Shader> gaussianBlurShader;
	static Ref<Shader> hdrShader;
	static Ref<Shader> bloomShader;
	static Ref<VertexArray> quadVertexArray;
	static Ref<VertexArray> cubeVertexArray;
	static Ref<UniformBuffer> ubCamera;
	static Ref<UniformBuffer> ubLights;

	static glm::mat4 cameraView;
	static glm::mat4 cameraProjection;
	static Entity skylight;
	static std::vector<Entity> sceneLights;
	glm::mat4 dirLightView;
	glm::mat4 dirLightViewProj;

	Ref<Framebuffer> mainFramebuffer;
	Ref<Framebuffer> ssgiFramebuffer;
	Ref<Framebuffer> tempBlurFramebuffer;
	Ref<Framebuffer> Renderer3D::prefilteredFramebuffer;
	Ref<Framebuffer> Renderer3D::downsampledFramebuffers[blurSamples];
	Ref<Framebuffer> Renderer3D::upsampledFramebuffers[blurSamples];

	ShaderLibrary Renderer3D::s_ShaderLibrary;
	Renderer3D::TonemappingType Renderer3D::Tonemapping = Renderer3D::TonemappingType::ACES;
	float Renderer3D::Exposure = 1.0f;
	bool Renderer3D::UseBloom = true;
	float Renderer3D::BloomStrength = 0.1f;
	float Renderer3D::BloomThreshold = 1.0f;
	float Renderer3D::BloomKnee = 0.1f;
	float Renderer3D::BloomClamp = 100.0f;

	void Renderer3D::Init()
	{
		uint32_t width = 1280;
		uint32_t height = 720;

		FramebufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::Depth };
		spec.Width = width;
		spec.Height = height;
		mainFramebuffer = Framebuffer::Create(spec);

		FramebufferSpecification ssgiSpec;
		ssgiSpec.Attachments = { FramebufferTextureFormat::RGBA16F };
		ssgiSpec.Width = width;
		ssgiSpec.Height = height;
		ssgiFramebuffer = Framebuffer::Create(ssgiSpec);

		width /= 2;
		height /= 2;
		FramebufferSpecification bloomSpec;
		bloomSpec.Attachments = { FramebufferTextureFormat::RGBA16F };
		bloomSpec.Width = width;
		bloomSpec.Height = height;
		prefilteredFramebuffer = Framebuffer::Create(bloomSpec);
		tempBlurFramebuffer = Framebuffer::Create(bloomSpec);
		
		for (size_t i = 0; i < blurSamples; i++)
		{
			width /= 2;
			height /= 2;
			FramebufferSpecification blurSpec;
			blurSpec.Attachments = { FramebufferTextureFormat::RGBA16F };
			blurSpec.Width = width;
			blurSpec.Height = height;
			downsampledFramebuffers[i] = Framebuffer::Create(blurSpec);
			upsampledFramebuffers[i] = Framebuffer::Create(blurSpec);
		}
		
		
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
		gaussianBlurShader = s_ShaderLibrary.Load("assets/shaders/GaussianBlur.glsl");
		hdrShader = s_ShaderLibrary.Load("assets/shaders/HDR.glsl");
		bloomShader = s_ShaderLibrary.Load("assets/shaders/Bloom.glsl");
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

		// Quad
		{
			float vertices[20] = {
				 -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
			};

			uint32_t indices[6] = {
				0, 1, 2,
				0, 2, 3
			};

			quadVertexArray = VertexArray::Create();

			Ref<VertexBuffer> quadVertexBuffer = VertexBuffer::Create(vertices, 20 * sizeof(float));
			quadVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			});
			quadVertexArray->AddVertexBuffer(quadVertexBuffer);

			Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(indices, 6);
			quadVertexArray->SetIndexBuffer(quadIndexBuffer);
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

	void DrawQuad()
	{
		RenderCommand::DrawIndexed(quadVertexArray);
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
		mesh.UseMRAMap = meshComponent.UseMRAMap;
		mesh.UseEmissiveMap = meshComponent.UseEmissiveMap;

		mesh.AlbedoMap = meshComponent.AlbedoMap;
		mesh.NormalMap = meshComponent.NormalMap;
		mesh.MRAMap = meshComponent.MRAMap;
		mesh.UseEmissiveMap = meshComponent.UseEmissiveMap;
		
		meshes.push_back(mesh);
	}

	void Renderer3D::Flush(Ref<Framebuffer>& renderTarget)
	{
		ARC_PROFILE_FUNCTION();

		auto& targetSpec = renderTarget->GetSpecification();
		auto& mainSpec = mainFramebuffer->GetSpecification();
		if (mainSpec.Width != targetSpec.Width || mainSpec.Height != targetSpec.Height)
		{
			uint32_t width = targetSpec.Width;
			uint32_t height = targetSpec.Height;
			mainFramebuffer->Resize(width, height);
			ssgiFramebuffer->Resize(width, height);

			width /= 2;
			height /= 2;
			prefilteredFramebuffer->Resize(width, height);

			for (size_t i = 0; i < blurSamples; i++)
			{
				width /= 2;
				height /= 2;
				downsampledFramebuffers[i]->Resize(width, height);
				upsampledFramebuffers[i]->Resize(width, height);
			}
		}

		ShadowMapPass();
		RenderPass();
		SSGIPass();
		BloomPass();
		CompositePass(renderTarget);
		meshes.clear();
	}

	void Renderer3D::SetupCameraData(const EditorCamera& camera)
	{
		ubCamera->Bind();

		uint32_t offset = 0;

		ubCamera->SetData((void*)glm::value_ptr(camera.GetViewMatrix()), offset, sizeof(glm::mat4));
		offset += sizeof(glm::mat4);

		ubCamera->SetData((void*)glm::value_ptr(camera.GetProjection()), offset, sizeof(glm::mat4));
		offset += sizeof(glm::mat4);

		ubCamera->SetData((void*)glm::value_ptr(camera.GetViewProjection()), offset, sizeof(glm::mat4));
		offset += sizeof(glm::mat4);
		
		ubCamera->SetData((void*)glm::value_ptr(camera.GetPosition()), offset, sizeof(glm::vec4));
		offset += sizeof(glm::vec4);
	}

	void Renderer3D::SetupLightsData()
	{
		uint32_t numLights = 0;
		uint32_t size = 5 * sizeof(glm::vec4);
		ubLights->Bind();
		for (Entity e : sceneLights)
		{
			TransformComponent transformComponent = e.GetComponent<TransformComponent>();
			LightComponent& lightComponent = e.GetComponent<LightComponent>();
			glm::mat4& worldTransform = e.GetWorldTransform();
			glm::vec4 attenFactors = glm::vec4(
				lightComponent.Radius,
				lightComponent.Falloff,
				0.0f,
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

	void Renderer3D::CompositePass(Ref<Framebuffer>& renderTarget)
	{
		renderTarget->Bind();
		hdrShader->Bind();
		glm::vec4 tonemappingParams = glm::vec4(((int) Tonemapping), Exposure, 0.0f, 0.0f);
		hdrShader->SetFloat4("u_TonemappParams", tonemappingParams);
		hdrShader->SetFloat("u_BloomStrength", UseBloom ? BloomStrength : 0.0f);
		hdrShader->SetInt("u_Texture", 0);
		hdrShader->SetInt("u_BloomTexture", 1);
		mainFramebuffer->BindColorAttachment(0, 0);
		upsampledFramebuffers[blurSamples - 1]->BindColorAttachment(0, 1);
		DrawQuad();
		renderTarget->Unbind();
	}

	void Renderer3D::BloomPass()
	{
		if (!UseBloom)
			return;
		
		prefilteredFramebuffer->Bind();
		bloomShader->Bind();
		glm::vec4 threshold = glm::vec4(BloomThreshold, BloomKnee, BloomKnee * 2.0f, 0.25f / BloomKnee);
		bloomShader->SetFloat4("u_Threshold", threshold);
		glm::vec4 params = glm::vec4(BloomClamp, 2.0f, 0.0f, 0.0f);
		bloomShader->SetFloat4("u_Params", params);
		bloomShader->SetInt("u_Texture", 0);
		mainFramebuffer->BindColorAttachment(0, 0);
		DrawQuad();

		FramebufferSpecification spec = prefilteredFramebuffer->GetSpecification();
		uint32_t width = spec.Width;
		uint32_t height = spec.Height;
		gaussianBlurShader->Bind();
		gaussianBlurShader->SetInt("u_Texture", 0);
		for (size_t i = 0; i < blurSamples; i++)
		{
			width /= 2;
			height /= 2;

			tempBlurFramebuffer->Resize(width, height);
			tempBlurFramebuffer->Bind();
			gaussianBlurShader->SetInt("u_Horizontal", static_cast<int>(true));
			if (i == 0)
				prefilteredFramebuffer->BindColorAttachment(0, 0);
			else
				downsampledFramebuffers[i - 1]->BindColorAttachment(0, 0);
			DrawQuad();

			downsampledFramebuffers[i]->Bind();
			gaussianBlurShader->SetInt("u_Horizontal", static_cast<int>(false));
			tempBlurFramebuffer->BindColorAttachment(0, 0);
			DrawQuad();
		}
		
		bloomShader->Bind();
		bloomShader->SetFloat4("u_Threshold", threshold);
		params = glm::vec4(BloomClamp, 3.0f, 1.0f, 1.0f);
		bloomShader->SetFloat4("u_Params", params);
		bloomShader->SetInt("u_Texture", 0);
		bloomShader->SetInt("u_AdditiveTexture", 1);
		size_t upsampleCount = blurSamples - 1;
		for (size_t i = 0; i < upsampleCount; i++)
		{
			const auto& spec = downsampledFramebuffers[upsampleCount - 1 - i]->GetSpecification();
			upsampledFramebuffers[i]->Resize(spec.Width, spec.Height);
			upsampledFramebuffers[i]->Bind();
			
			if (i == 0)
			{
				downsampledFramebuffers[upsampleCount - i]->BindColorAttachment(0, 0); // 5
				downsampledFramebuffers[upsampleCount - 1 - i]->BindColorAttachment(0, 1); // 4
			}
			else
			{
				upsampledFramebuffers[i - 1]->BindColorAttachment(0, 0); // 5
				downsampledFramebuffers[upsampleCount - 1 - i]->BindColorAttachment(0, 1); // 3
			}
			DrawQuad();
		}
		
		upsampledFramebuffers[upsampleCount]->Resize(spec.Width, spec.Height);
		upsampledFramebuffers[upsampleCount]->Bind();
		params = glm::vec4(BloomClamp, 3.0f, 1.0f, 0.0f);
		bloomShader->SetFloat4("u_Params", params);
		upsampledFramebuffers[upsampleCount - 1]->BindColorAttachment(0, 0);
		DrawQuad();
		upsampledFramebuffers[upsampleCount]->Unbind();
	}

	void Renderer3D::SSGIPass()
	{

	}

	void Renderer3D::RenderPass()
	{
		mainFramebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

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
			shader->SetInt("u_UseMRAMap", static_cast<int>(meshData->UseMRAMap));
			shader->SetInt("u_UseEmissiveMap", static_cast<int>(meshData->UseEmissiveMap));

			shader->SetInt("u_IrradianceMap", 0);
			shader->SetInt("u_DirectionalShadowMap", 1);

			shader->SetInt("u_AlbedoMap", 2);
			shader->SetInt("u_NormalMap", 3);
			shader->SetInt("u_MRAMap", 4);
			shader->SetInt("u_EmissiveMap", 5);

			for (size_t i = 0; i < sceneLights.size(); i++)
				sceneLights[i].GetComponent<LightComponent>().ShadowMapFramebuffer->BindDepthAttachment(i + 1);

			if (meshData->AlbedoMap)
				meshData->AlbedoMap->Bind(2);
			if (meshData->NormalMap)
				meshData->NormalMap->Bind(3);
			if (meshData->MRAMap)
				meshData->MRAMap->Bind(4);
			if (meshData->EmissiveMap)
				meshData->EmissiveMap->Bind(5);
			
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

		mainFramebuffer->Unbind();
	}

	void Renderer3D::ShadowMapPass()
	{
//		RenderCommand::FrontCull();
		for (size_t i = 0; i < sceneLights.size(); i++)
		{
			Entity e = sceneLights[i];
			LightComponent& light = e.GetComponent<LightComponent>();
			if (light.Type != LightComponent::LightType::Directional)
				continue;

			light.ShadowMapFramebuffer->Bind();
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

			light.ShadowMapFramebuffer->Unbind();
			
			break;
		}
//		RenderCommand::BackCull();
	}
}
