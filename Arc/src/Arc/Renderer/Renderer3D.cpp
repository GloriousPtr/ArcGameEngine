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
		/*
		* Properties[0] = AlbedoColor: r, g, b, a;
		* Properties[1] = Metallic, Roughness, unused, unused
		* Properties[2] = EmissiveParams: r, g, b, intensity
		* Properties[4] = UseAlbedMap, UseNormalMap, UseMRAMap, UseEmissiveMap
		*/
		glm::mat4 Properties = glm::mat4(0.0f);
		glm::mat4 Transform = glm::mat4(1.0f);
		Ref<VertexArray> VertexArray = nullptr;
		
		MeshComponent::CullModeType CullMode;

		Ref<Texture2D> AlbedoMap = nullptr;
		Ref<Texture2D> NormalMap = nullptr;
		Ref<Texture2D> MRAMap = nullptr;
		Ref<Texture2D> EmissiveMap = nullptr;
	};

	Renderer3D::Statistics s_Stats;
	static std::vector<MeshData> meshes;
	static Ref<Texture2D> s_BRDFLutTexture;
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
		OPTICK_EVENT();

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

		s_BRDFLutTexture = Texture2D::Create("Resources/Renderer/BRDF_LUT.tga");

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
		OPTICK_EVENT();

	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

	}

	void Renderer3D::BeginScene(const EditorCamera& camera, Entity cubemap, std::vector<Entity>& lights)
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();
		
		cameraView = camera.GetViewMatrix();
		cameraProjection = camera.GetProjection();

		skylight = cubemap;
		sceneLights = lights;

		SetupCameraData(camera);
		SetupLightsData();
	}

	void Renderer3D::EndScene(Ref<RenderGraphData>& renderTarget)
	{
		OPTICK_EVENT();

		Flush(renderTarget);

		shader->Unbind();
	}

	void Renderer3D::DrawCube()
	{
		OPTICK_EVENT();

		RenderCommand::Draw(cubeVertexArray, 36);
	}

	void Renderer3D::DrawQuad()
	{
		OPTICK_EVENT();

		RenderCommand::DrawIndexed(quadVertexArray);
	}

	void Renderer3D::SubmitMesh(MeshComponent& meshComponent, const glm::mat4& transform)
	{
		OPTICK_EVENT();

		ARC_PROFILE_FUNCTION();

		MeshData mesh;

		mesh.Properties = glm::mat4(
			meshComponent.AlbedoColor,
			meshComponent.MR,
			meshComponent.EmissiveParams,
			meshComponent.UseMaps);

		mesh.VertexArray = meshComponent.VertexArray;
		mesh.Transform = transform;
		mesh.CullMode = meshComponent.CullMode;

		mesh.AlbedoMap = meshComponent.AlbedoMap;
		mesh.NormalMap = meshComponent.NormalMap;
		mesh.MRAMap = meshComponent.MRAMap;
		mesh.EmissiveMap = meshComponent.EmissiveMap;
		
		meshes.push_back(mesh);
	}

	void Renderer3D::Flush(Ref<RenderGraphData> renderGraphData)
	{
		OPTICK_EVENT();
		
		ARC_PROFILE_FUNCTION();
		
		ShadowMapPass();
		RenderPass(renderGraphData->RenderPassTarget);
		BloomPass(renderGraphData);
		CompositePass(renderGraphData);
		meshes.clear();
	}

	void Renderer3D::ResetStats()
	{
		memset(&s_Stats, 0, sizeof(Statistics));
	}

	Renderer3D::Statistics Renderer3D::GetStats()
	{
		return s_Stats;
	}

	void Renderer3D::SetupCameraData(const EditorCamera& camera)
	{
		OPTICK_EVENT();

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
		OPTICK_EVENT();

		struct LightData
		{
			glm::vec4 Position;
			glm::vec4 Color;
			glm::vec4 AttenFactors;
			glm::vec4 LightDir;
		};

		uint32_t numLights = 0;
		const uint32_t size = sizeof(LightData);

		ubLights->Bind();

		for (Entity e : sceneLights)
		{
			TransformComponent transformComponent = e.GetComponent<TransformComponent>();
			LightComponent& lightComponent = e.GetComponent<LightComponent>();
			glm::mat4& worldTransform = e.GetWorldTransform();
			
			glm::vec4 attenFactors = glm::vec4(
				lightComponent.Range,
				glm::cos(glm::radians(lightComponent.CutOffAngle)),
				glm::cos(glm::radians(lightComponent.OuterCutOffAngle)),
				static_cast<uint32_t>(lightComponent.Type));
			// Based off of +Z direction
			glm::vec4 zDir = worldTransform * glm::vec4(0, 0, 1, 0);

			LightData lightData = 
			{
				worldTransform[3],
				glm::vec4(lightComponent.Color, lightComponent.Intensity),
				attenFactors,
				zDir
			};

			ubLights->SetData((void*)(&lightData), size * numLights, size);

			numLights++;
		}

		// Pass number of lights within the scene
		// 25 is max number of lights
		ubLights->SetData(&numLights, 25 * size, sizeof(uint32_t));
	}

	void Renderer3D::CompositePass(Ref<RenderGraphData> renderGraphData)
	{
		OPTICK_EVENT();

		renderGraphData->CompositePassTarget->Bind();
		hdrShader->Bind();
		glm::vec4 tonemappingParams = glm::vec4(((int) Tonemapping), Exposure, 0.0f, 0.0f);
		hdrShader->SetFloat4("u_TonemappParams", tonemappingParams);
		hdrShader->SetFloat("u_BloomStrength", UseBloom ? BloomStrength : 0.0f);
		hdrShader->SetInt("u_Texture", 0);
		hdrShader->SetInt("u_BloomTexture", 1);
		renderGraphData->RenderPassTarget->BindColorAttachment(0, 0);
		renderGraphData->UpsampledFramebuffers[0]->BindColorAttachment(0, 1);
		DrawQuad();
	}

	void Renderer3D::BloomPass(Ref<RenderGraphData> renderGraphData)
	{
		OPTICK_EVENT();

		if (!UseBloom)
			return;
		
		glm::vec4 threshold = glm::vec4(BloomThreshold, BloomKnee, BloomKnee * 2.0f, 0.25f / BloomKnee);
		glm::vec4 params = glm::vec4(BloomClamp, 2.0f, 0.0f, 0.0f);

		{
			OPTICK_EVENT("Prefilter");

			renderGraphData->PrefilteredFramebuffer->Bind();
			bloomShader->Bind();
			bloomShader->SetFloat4("u_Threshold", threshold);
			bloomShader->SetFloat4("u_Params", params);
			bloomShader->SetInt("u_Texture", 0);
			renderGraphData->RenderPassTarget->BindColorAttachment(0, 0);
			DrawQuad();
		}

		size_t blurSamples = renderGraphData->BlurSamples;
		FramebufferSpecification spec = renderGraphData->PrefilteredFramebuffer->GetSpecification();
		{
			OPTICK_EVENT("Downsample");

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
			OPTICK_EVENT("Upsample");

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

	void Renderer3D::RenderPass(Ref<Framebuffer> renderTarget)
	{
		OPTICK_EVENT();

		renderTarget->Bind();
		RenderCommand::Clear();

		float skylightIntensity = 0.0f;
		float skylightRotation = 0.0f;
		{
			OPTICK_EVENT("Skylight");

			SkyLightComponent* skylightComponent = nullptr;
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

					skylightComponent->Texture->BindIrradianceMap(0);
					skylightComponent->Texture->BindRadianceMap(1);
					s_BRDFLutTexture->Bind(2);
				}
			}
		}

		if (meshes.size() > 0)
		{
			OPTICK_EVENT("Shader Binding");

			shader->Bind();
			shader->SetFloat("u_IrradianceIntensity", skylightIntensity);
			shader->SetFloat("u_EnvironmentRotation", skylightRotation);
			shader->SetInt("u_IrradianceMap", 0);
			shader->SetInt("u_RadianceMap", 1);
			shader->SetInt("u_BRDFLutMap", 2);
			shader->SetInt("u_DirectionalShadowMap", 3);
			shader->SetInt("u_AlbedoMap", 4);
			shader->SetInt("u_NormalMap", 5);
			shader->SetInt("u_MRAMap", 6);
			shader->SetInt("u_EmissiveMap", 7);

			shader->SetMat4("u_DirLightView", dirLightView);
			shader->SetMat4("u_DirLightViewProj", dirLightViewProj);
		}

		{
			OPTICK_EVENT("Draw Meshes");

			for (size_t i = 0; i < sceneLights.size(); i++)
				sceneLights[i].GetComponent<LightComponent>().ShadowMapFramebuffer->BindDepthAttachment(i + 3);

			MeshComponent::CullModeType currentCullMode = MeshComponent::CullModeType::Unknown;
			for (auto it = meshes.rbegin(); it != meshes.rend(); it++)
			{
				MeshData* meshData = &(*it);

				shader->SetMat4("u_Properties", meshData->Properties);

				if (meshData->AlbedoMap)
					meshData->AlbedoMap->Bind(4);
				if (meshData->NormalMap)
					meshData->NormalMap->Bind(5);
				if (meshData->MRAMap)
					meshData->MRAMap->Bind(6);
				if (meshData->EmissiveMap)
					meshData->EmissiveMap->Bind(7);
			
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

				RenderCommand::DrawIndexed(meshData->VertexArray);
				s_Stats.DrawCalls++;
				s_Stats.IndexCount += meshData->VertexArray->GetIndexBuffer()->GetCount();
			}
		}
	}

	void Renderer3D::ShadowMapPass()
	{
		OPTICK_EVENT();

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
			
			break;
		}
//		RenderCommand::BackCull();
	}
}
