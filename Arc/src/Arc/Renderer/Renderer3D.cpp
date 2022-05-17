#include "arcpch.h"
#include "Renderer3D.h"

#include "Renderer.h"
#include "Framebuffer.h"
#include "Arc/Renderer/Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ArcEngine
{
	const static uint32_t MAX_NUM_LIGHTS = 200;
	const static uint32_t MAX_NUM_DIR_LIGHTS = 3;

	struct MeshData
	{
		glm::mat4 Transform;
		Submesh& Submesh;
		MeshComponent::CullModeType CullMode;
	};

	Renderer3D::Statistics s_Stats;
	static std::vector<MeshData> meshes;
	static Ref<Texture2D> s_BRDFLutTexture;
	static Ref<Shader> shader;
	static Ref<Shader> lightingShader;
	static Ref<Shader> shadowMapShader;
	static Ref<Shader> cubemapShader;
	static Ref<Shader> gaussianBlurShader;
	static Ref<Shader> hdrShader;
	static Ref<Shader> bloomShader;
	static Ref<VertexArray> quadVertexArray;
	static Ref<VertexArray> cubeVertexArray;
	static Ref<UniformBuffer> ubCamera;
	static Ref<UniformBuffer> ubPointLights;
	static Ref<UniformBuffer> ubDirectionalLights;

	static glm::mat4 cameraView;
	static glm::mat4 cameraProjection;
	static glm::vec3 cameraPosition;
	static Entity skylight;
	static std::vector<Entity> sceneLights;

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
		ARC_PROFILE_SCOPE();

		ubCamera = UniformBuffer::Create();
		ubCamera->SetLayout({
			{ ShaderDataType::Mat4, "u_View" },
			{ ShaderDataType::Mat4, "u_Projection" },
			{ ShaderDataType::Mat4, "u_ViewProjection" },
			{ ShaderDataType::Float4, "u_CameraPosition" }
		}, 0);

		ubPointLights = UniformBuffer::Create();
		ubPointLights->SetLayout({
			{ ShaderDataType::Float4, "u_Position" },
			{ ShaderDataType::Float4, "u_Color" },
			{ ShaderDataType::Float4, "u_AttenFactors" },
			{ ShaderDataType::Float4, "u_LightDir" },
		}, 1, MAX_NUM_LIGHTS + 1);

		ubDirectionalLights = UniformBuffer::Create();
		ubDirectionalLights->SetLayout({
			{ ShaderDataType::Float4, "u_Position" },
			{ ShaderDataType::Float4, "u_Color" },
			{ ShaderDataType::Float4, "u_LightDir" },
			{ ShaderDataType::Mat4, "u_DirLightViewProj" },
		}, 2, MAX_NUM_DIR_LIGHTS + 1);

		s_BRDFLutTexture = Texture2D::Create("Resources/Renderer/BRDF_LUT.jpg");

		s_ShaderLibrary = ShaderLibrary();
		shadowMapShader = s_ShaderLibrary.Load("assets/shaders/DepthShader.glsl");
		cubemapShader = s_ShaderLibrary.Load("assets/shaders/Cubemap.glsl");
		gaussianBlurShader = s_ShaderLibrary.Load("assets/shaders/GaussianBlur.glsl");
		hdrShader = s_ShaderLibrary.Load("assets/shaders/HDR.glsl");
		bloomShader = s_ShaderLibrary.Load("assets/shaders/Bloom.glsl");
		shader = s_ShaderLibrary.Load("assets/shaders/PBR.glsl");
		lightingShader = s_ShaderLibrary.Load("assets/shaders/LightingPass.glsl");

		shader->Bind();
		shader->SetUniformBlock("Camera", 0);

		lightingShader->Bind();
		lightingShader->SetUniformBlock("Camera", 0);
		lightingShader->SetUniformBlock("PointLightBuffer", 1);
		lightingShader->SetUniformBlock("DirectionalLightBuffer", 2);

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
		ARC_PROFILE_SCOPE();

	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform, Entity cubemap, std::vector<Entity>& lights)
	{
		ARC_PROFILE_SCOPE();
		
		cameraView = glm::inverse(transform);
		cameraProjection = camera.GetProjection();
		cameraPosition = transform[3];

		skylight = cubemap;
		sceneLights = lights;

		SetupCameraData();
		SetupLightsData();
	}

	void Renderer3D::BeginScene(const EditorCamera& camera, Entity cubemap, std::vector<Entity>& lights)
	{
		ARC_PROFILE_SCOPE();
		
		cameraView = camera.GetViewMatrix();
		cameraProjection = camera.GetProjection();
		cameraPosition = camera.GetPosition();

		skylight = cubemap;
		sceneLights = lights;

		SetupCameraData();
		SetupLightsData();
	}

	void Renderer3D::EndScene(Ref<RenderGraphData>& renderTarget)
	{
		ARC_PROFILE_SCOPE();

		Flush(renderTarget);

		shader->Unbind();
	}

	void Renderer3D::DrawCube()
	{
		ARC_PROFILE_SCOPE();

		RenderCommand::Draw(cubeVertexArray, 36);
	}

	void Renderer3D::DrawQuad()
	{
		ARC_PROFILE_SCOPE();

		RenderCommand::DrawIndexed(quadVertexArray);
	}

	void Renderer3D::SubmitMesh(MeshComponent& meshComponent, const glm::mat4& transform)
	{
		ARC_PROFILE_SCOPE();

		if (meshComponent.MeshGeometry->GetSubmeshCount() == 0)
			return;

		ARC_CORE_ASSERT(meshComponent.MeshGeometry->GetSubmeshCount() > meshComponent.SubmeshIndex, "Trying to access submesh index that does not exist!");

		meshes.push_back({ transform, meshComponent.MeshGeometry->GetSubmesh(meshComponent.SubmeshIndex), meshComponent.CullMode });
	}

	void Renderer3D::Flush(Ref<RenderGraphData> renderGraphData)
	{
		ARC_PROFILE_SCOPE();
		
		ShadowMapPass();
		RenderPass(renderGraphData->RenderPassTarget);
		LightingPass(renderGraphData);
		BloomPass(renderGraphData);
		CompositePass(renderGraphData);
		meshes.clear();
	}

	void Renderer3D::ResetStats()
	{
		ARC_PROFILE_SCOPE();

		memset(&s_Stats, 0, sizeof(Statistics));
	}

	Renderer3D::Statistics Renderer3D::GetStats()
	{
		ARC_PROFILE_SCOPE();

		return s_Stats;
	}

	void Renderer3D::SetupCameraData()
	{
		ARC_PROFILE_SCOPE();

		ubCamera->Bind();

		uint32_t offset = 0;

		ubCamera->SetData((void*)glm::value_ptr(cameraView), offset, sizeof(glm::mat4));
		offset += sizeof(glm::mat4);

		ubCamera->SetData((void*)glm::value_ptr(cameraProjection), offset, sizeof(glm::mat4));
		offset += sizeof(glm::mat4);

		ubCamera->SetData((void*)glm::value_ptr(cameraProjection * cameraView), offset, sizeof(glm::mat4));
		offset += sizeof(glm::mat4);
		
		ubCamera->SetData((void*)glm::value_ptr(cameraPosition), offset, sizeof(glm::vec4));
		offset += sizeof(glm::vec4);
	}

	void Renderer3D::SetupLightsData()
	{
		ARC_PROFILE_SCOPE();

		{
			struct PointLightData
			{
				glm::vec4 Position;
				glm::vec4 Color;
				glm::vec4 AttenFactors;
				glm::vec4 LightDir;
			};

			uint32_t numLights = 0;
			const uint32_t size = sizeof(PointLightData);

			ubPointLights->Bind();

			for (Entity e : sceneLights)
			{
				LightComponent& lightComponent = e.GetComponent<LightComponent>();
				if (lightComponent.Type == LightComponent::LightType::Directional)
					continue;
				TransformComponent transformComponent = e.GetComponent<TransformComponent>();
				glm::mat4& worldTransform = e.GetWorldTransform();
			
				glm::vec4 attenFactors = glm::vec4(
					lightComponent.Range,
					glm::cos(glm::radians(lightComponent.CutOffAngle)),
					glm::cos(glm::radians(lightComponent.OuterCutOffAngle)),
					static_cast<uint32_t>(lightComponent.Type));
				// Based off of +Z direction
				glm::vec4 zDir = worldTransform * glm::vec4(0, 0, 1, 0);

				PointLightData pointLightData = 
				{
					worldTransform[3],
					glm::vec4(lightComponent.Color, lightComponent.Intensity),
					attenFactors,
					zDir
				};

				ubPointLights->SetData((void*)(&pointLightData), size * numLights, size);

				numLights++;
			}

			// Pass number of lights within the scene
			ubPointLights->SetData(&numLights, MAX_NUM_LIGHTS * size, sizeof(uint32_t));
		}

		{
			struct DirectionalLightData
			{
				glm::vec4 Position;
				glm::vec4 Color;
				glm::vec4 LightDir;
				glm::mat4 DirLightViewProj;
			};

			uint32_t numLights = 0;
			const uint32_t size = sizeof(DirectionalLightData);

			ubDirectionalLights->Bind();

			for (Entity e : sceneLights)
			{
				LightComponent& lightComponent = e.GetComponent<LightComponent>();
				if (lightComponent.Type != LightComponent::LightType::Directional)
					continue;

				TransformComponent transformComponent = e.GetComponent<TransformComponent>();
				glm::mat4& worldTransform = e.GetWorldTransform();
			
				// Based off of +Z direction
				glm::vec4 zDir = worldTransform * glm::vec4(0, 0, 1, 0);
				float near_plane = -100.0f, far_plane = 100.0f;
				glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
				glm::vec3 pos = worldTransform[3];
				glm::vec3 dir = glm::normalize(glm::vec3(zDir));
				glm::vec3 lookAt = pos + dir;
				glm::mat4 dirLightView = glm::lookAt(pos, lookAt, glm::vec3(0, 1 ,0));
				glm::mat4 dirLightViewProj = lightProjection * dirLightView;

				DirectionalLightData dirLightData = 
				{
					glm::vec4(pos, (uint32_t) lightComponent.ShadowQuality),
					glm::vec4(lightComponent.Color, lightComponent.Intensity),
					zDir,
					dirLightViewProj
				};

				ubDirectionalLights->SetData((void*)(&dirLightData), size * numLights, size);

				numLights++;
			}

			// Pass number of lights within the scene
			ubDirectionalLights->SetData(&numLights, MAX_NUM_DIR_LIGHTS * size, sizeof(uint32_t));
		}
	}

	void Renderer3D::CompositePass(Ref<RenderGraphData> renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		renderGraphData->CompositePassTarget->Bind();
		hdrShader->Bind();
		glm::vec4 tonemappingParams = glm::vec4(((int) Tonemapping), Exposure, 0.0f, 0.0f);
		hdrShader->SetFloat4("u_TonemappParams", tonemappingParams);
		hdrShader->SetFloat("u_BloomStrength", UseBloom ? BloomStrength : 0.0f);
		hdrShader->SetInt("u_Texture", 0);
		hdrShader->SetInt("u_BloomTexture", 1);
		renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);
		renderGraphData->UpsampledFramebuffers[0]->BindColorAttachment(0, 1);
		DrawQuad();
	}

	void Renderer3D::BloomPass(Ref<RenderGraphData> renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		if (!UseBloom)
			return;
		
		glm::vec4 threshold = glm::vec4(BloomThreshold, BloomKnee, BloomKnee * 2.0f, 0.25f / BloomKnee);
		glm::vec4 params = glm::vec4(BloomClamp, 2.0f, 0.0f, 0.0f);

		{
			ARC_PROFILE_SCOPE("Prefilter");

			renderGraphData->PrefilteredFramebuffer->Bind();
			bloomShader->Bind();
			bloomShader->SetFloat4("u_Threshold", threshold);
			bloomShader->SetFloat4("u_Params", params);
			bloomShader->SetInt("u_Texture", 0);
			renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);
			DrawQuad();
		}

		size_t blurSamples = renderGraphData->BlurSamples;
		FramebufferSpecification spec = renderGraphData->PrefilteredFramebuffer->GetSpecification();
		{
			ARC_PROFILE_SCOPE("Downsample");

			gaussianBlurShader->Bind();
			gaussianBlurShader->SetInt("u_Texture", 0);
			for (size_t i = 0; i < blurSamples; i++)
			{
				renderGraphData->TempBlurFramebuffers[i]->Bind();
				gaussianBlurShader->SetInt("u_Horizontal", static_cast<int>(true));
				if (i == 0)
					renderGraphData->PrefilteredFramebuffer->BindColorAttachment(0, 0);
				else
					renderGraphData->DownsampledFramebuffers[i - 1]->BindColorAttachment(0, 0);
				DrawQuad();

				renderGraphData->DownsampledFramebuffers[i]->Bind();
				gaussianBlurShader->SetInt("u_Horizontal", static_cast<int>(false));
				renderGraphData->TempBlurFramebuffers[i]->BindColorAttachment(0, 0);
				DrawQuad();
			}
		}
		
		{
			ARC_PROFILE_SCOPE("Upsample");

			bloomShader->Bind();
			bloomShader->SetFloat4("u_Threshold", threshold);
			params = glm::vec4(BloomClamp, 3.0f, 1.0f, 1.0f);
			bloomShader->SetFloat4("u_Params", params);
			bloomShader->SetInt("u_Texture", 0);
			bloomShader->SetInt("u_AdditiveTexture", 1);
			size_t upsampleCount = blurSamples - 1;
			for (size_t i = upsampleCount; i > 0; i--)
			{
				renderGraphData->UpsampledFramebuffers[i]->Bind();
			
				if (i == upsampleCount)
				{
					renderGraphData->DownsampledFramebuffers[upsampleCount]->BindColorAttachment(0, 0);
					renderGraphData->DownsampledFramebuffers[upsampleCount - 1]->BindColorAttachment(0, 1);
				}
				else
				{
					renderGraphData->DownsampledFramebuffers[i]->BindColorAttachment(0, 0);
					renderGraphData->UpsampledFramebuffers[i + 1]->BindColorAttachment(0, 1);
				}
				DrawQuad();
			}
		
			renderGraphData->UpsampledFramebuffers[0]->Bind();
			params = glm::vec4(BloomClamp, 3.0f, 1.0f, 0.0f);
			bloomShader->SetFloat4("u_Params", params);
			renderGraphData->UpsampledFramebuffers[1]->BindColorAttachment(0, 0);
			DrawQuad();
		}
	}

	void Renderer3D::LightingPass(Ref<RenderGraphData> renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		renderGraphData->LightingPassTarget->Bind();
		RenderCommand::Clear();

		lightingShader->Bind();

		lightingShader->SetInt("u_Albedo", 0);
		lightingShader->SetInt("u_Normal", 1);
		lightingShader->SetInt("u_MetallicRoughnessAO", 2);
		lightingShader->SetInt("u_Emission", 3);
		lightingShader->SetInt("u_Depth", 4);

		lightingShader->SetInt("u_IrradianceMap", 5);
		lightingShader->SetInt("u_RadianceMap", 6);
		lightingShader->SetInt("u_BRDFLutMap", 7);

		int32_t samplers[MAX_NUM_DIR_LIGHTS];
		for (uint32_t i = 0; i < MAX_NUM_DIR_LIGHTS; i++)
			samplers[i] = i + 8;
		lightingShader->SetIntArray("u_DirectionalShadowMap", samplers, MAX_NUM_DIR_LIGHTS);

		renderGraphData->RenderPassTarget->BindColorAttachment(0, 0);
		renderGraphData->RenderPassTarget->BindColorAttachment(1, 1);
		renderGraphData->RenderPassTarget->BindColorAttachment(2, 2);
		renderGraphData->RenderPassTarget->BindColorAttachment(3, 3);
		renderGraphData->RenderPassTarget->BindDepthAttachment(4);
		
		if (skylight)
		{
			SkyLightComponent& skylightComponent = skylight.GetComponent<SkyLightComponent>();
			if (skylightComponent.Texture)
			{
				lightingShader->SetFloat("u_IrradianceIntensity", skylightComponent.Intensity);
				lightingShader->SetFloat("u_EnvironmentRotation", skylightComponent.Rotation);
				skylightComponent.Texture->BindIrradianceMap(5);
				skylightComponent.Texture->BindRadianceMap(6);
			}
		}
		s_BRDFLutTexture->Bind(7);
		
		uint32_t dirLightIndex = 0;
		for (size_t i = 0; i < sceneLights.size(); i++)
		{
			LightComponent& light = sceneLights[i].GetComponent<LightComponent>();
			if (light.Type == LightComponent::LightType::Directional)
			{
				if (dirLightIndex < MAX_NUM_DIR_LIGHTS)
				{
					light.ShadowMapFramebuffer->BindDepthAttachment(8 + dirLightIndex);
					dirLightIndex++;
				}
				else
				{
					break;
				}
			}
		}
		
		DrawQuad();
	}

	void Renderer3D::RenderPass(Ref<Framebuffer> renderTarget)
	{
		ARC_PROFILE_SCOPE();

		renderTarget->Bind();
		RenderCommand::SetBlendState(false);
		RenderCommand::SetClearColor(glm::vec4(0.0f));
		RenderCommand::Clear();

		SkyLightComponent* skylightComponent = nullptr;
		float skylightIntensity = 0.0f;
		float skylightRotation = 0.0f;
		{
			ARC_PROFILE_SCOPE("Skylight");

			if (skylight)
			{
				skylightComponent = &(skylight.GetComponent<SkyLightComponent>());
				if (skylightComponent->Texture)
				{
					RenderCommand::SetDepthMask(false);

					skylightIntensity = skylightComponent->Intensity;
					skylightRotation = skylightComponent->Rotation;

					skylightComponent->Texture->Bind(0);
					cubemapShader->Bind();
					cubemapShader->SetMat4("u_View", cameraView);
					cubemapShader->SetMat4("u_Projection", cameraProjection);
					cubemapShader->SetFloat("u_Intensity", skylightIntensity);
					cubemapShader->SetFloat("u_Rotation", skylightRotation);

					DrawCube();

					RenderCommand::SetDepthMask(true);
				}
			}
		}
		
		{
			ARC_PROFILE_SCOPE("Draw Meshes");

			MeshComponent::CullModeType currentCullMode = MeshComponent::CullModeType::Unknown;
			for (auto it = meshes.rbegin(); it != meshes.rend(); it++)
			{
				MeshData* meshData = &(*it);
				meshData->Submesh.Mat->Bind();
				shader->SetMat4("u_Model", meshData->Transform);
				
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

				RenderCommand::DrawIndexed(meshData->Submesh.Geometry);
				s_Stats.DrawCalls++;
				s_Stats.IndexCount += meshData->Submesh.Geometry->GetIndexBuffer()->GetCount();
			}
		}
	}

	void Renderer3D::ShadowMapPass()
	{
		ARC_PROFILE_SCOPE();

//		RenderCommand::FrontCull();
		for (size_t i = 0; i < sceneLights.size(); i++)
		{
			Entity e = sceneLights[i];
			LightComponent& light = e.GetComponent<LightComponent>();
			if (light.Type != LightComponent::LightType::Directional)
				continue;

			light.ShadowMapFramebuffer->Bind();
			RenderCommand::Clear();

			glm::mat4 transform = e.GetWorldTransform();
			float near_plane = -100.0f, far_plane = 100.0f;
			
			glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
			
			glm::vec4 zDir = transform * glm::vec4(0, 0, 1, 0);

			glm::vec3 pos = transform[3];
			glm::vec3 dir = glm::normalize(glm::vec3(zDir));
			glm::vec3 lookAt = pos + dir;
			glm::mat4 dirLightView = glm::lookAt(pos, lookAt, glm::vec3(0, 1 ,0));
			glm::mat4 dirLightViewProj = lightProjection * dirLightView;

			shadowMapShader->Bind();
			shadowMapShader->SetMat4("u_ViewProjection", dirLightViewProj);

			for (auto it = meshes.rbegin(); it != meshes.rend(); it++)
			{
				MeshData* meshData = &(*it);

				shadowMapShader->SetMat4("u_Model", meshData->Transform);
				RenderCommand::DrawIndexed(meshData->Submesh.Geometry);
			}
		}
//		RenderCommand::BackCull();
	}
}
