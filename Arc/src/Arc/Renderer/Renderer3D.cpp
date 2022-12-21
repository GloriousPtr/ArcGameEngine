#include "arcpch.h"
#include "Renderer3D.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include "RenderGraphData.h"
#include "Framebuffer.h"
#include "Arc/Renderer/VertexArray.h"
#include "Arc/Renderer/Material.h"

#include "Arc/Scene/Entity.h"

namespace ArcEngine
{
	Renderer3D::Statistics Renderer3D::s_Stats;
	std::vector<Renderer3D::MeshData> Renderer3D::s_Meshes;
	Ref<Texture2D> Renderer3D::s_BRDFLutTexture;
	Ref<Shader> Renderer3D::s_Shader;
	Ref<Shader> Renderer3D::s_LightingShader;
	Ref<Shader> Renderer3D::s_ShadowMapShader;
	Ref<Shader> Renderer3D::s_CubemapShader;
	Ref<Shader> Renderer3D::s_GaussianBlurShader;
	Ref<Shader> Renderer3D::s_FxaaShader;
	Ref<Shader> Renderer3D::s_HdrShader;
	Ref<Shader> Renderer3D::s_BloomShader;
	Ref<VertexArray> Renderer3D::s_QuadVertexArray;
	Ref<VertexArray> Renderer3D::s_CubeVertexArray;
	Ref<UniformBuffer> Renderer3D::s_UbCamera;
	Ref<UniformBuffer> Renderer3D::s_UbPointLights;
	Ref<UniformBuffer> Renderer3D::s_UbDirectionalLights;

	Entity Renderer3D::s_Skylight;
	std::vector<Entity> Renderer3D::s_SceneLights;

	ShaderLibrary Renderer3D::s_ShaderLibrary;
	Renderer3D::TonemappingType Renderer3D::Tonemapping = Renderer3D::TonemappingType::ACES;
	float Renderer3D::Exposure = 1.0f;
	bool Renderer3D::UseBloom = true;
	float Renderer3D::BloomStrength = 0.1f;
	float Renderer3D::BloomThreshold = 1.0f;
	float Renderer3D::BloomKnee = 0.1f;
	float Renderer3D::BloomClamp = 100.0f;
	bool Renderer3D::UseFXAA = true;
	glm::vec2 Renderer3D::FXAAThreshold = glm::vec2(0.0078125f, 0.125f);		// x: current threshold, y: relative threshold
	glm::vec4 Renderer3D::VignetteColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.25f);	// rgb: color, a: intensity
	glm::vec4 Renderer3D::VignetteOffset = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);	// xy: offset, z: useMask, w: enable/disable effect
	Ref<Texture2D> Renderer3D::VignetteMask = nullptr;

	void Renderer3D::Init()
	{
		ARC_PROFILE_SCOPE();

		s_UbCamera = UniformBuffer::Create();
		s_UbCamera->SetLayout({
			{ ShaderDataType::Mat4, "u_View" },
			{ ShaderDataType::Mat4, "u_Projection" },
			{ ShaderDataType::Mat4, "u_ViewProjection" },
			{ ShaderDataType::Float4, "u_CameraPosition" }
		}, 0);

		s_UbPointLights = UniformBuffer::Create();
		s_UbPointLights->SetLayout({
			{ ShaderDataType::Float4, "u_Position" },
			{ ShaderDataType::Float4, "u_Color" },
			{ ShaderDataType::Float4, "u_AttenFactors" },
			{ ShaderDataType::Float4, "u_LightDir" },
		}, 1, MAX_NUM_LIGHTS + 1);

		s_UbDirectionalLights = UniformBuffer::Create();
		s_UbDirectionalLights->SetLayout({
			{ ShaderDataType::Float4, "u_Position" },
			{ ShaderDataType::Float4, "u_Color" },
			{ ShaderDataType::Float4, "u_LightDir" },
			{ ShaderDataType::Mat4, "u_DirLightViewProj" },
		}, 2, MAX_NUM_DIR_LIGHTS + 1);

		s_BRDFLutTexture = Texture2D::Create("Resources/Renderer/BRDF_LUT.jpg");

		s_ShaderLibrary = ShaderLibrary();
		s_ShadowMapShader = s_ShaderLibrary.Load("assets/shaders/DepthShader.glsl");
		s_CubemapShader = s_ShaderLibrary.Load("assets/shaders/Cubemap.glsl");
		s_GaussianBlurShader = s_ShaderLibrary.Load("assets/shaders/GaussianBlur.glsl");
		s_FxaaShader = s_ShaderLibrary.Load("assets/shaders/FXAA.glsl");
		s_HdrShader = s_ShaderLibrary.Load("assets/shaders/HDR.glsl");
		s_BloomShader = s_ShaderLibrary.Load("assets/shaders/Bloom.glsl");
		s_Shader = s_ShaderLibrary.Load("assets/shaders/PBR.glsl");
		s_LightingShader = s_ShaderLibrary.Load("assets/shaders/LightingPass.glsl");

		s_Shader->Bind();
		s_Shader->SetUniformBlock("Camera", 0);

		s_CubemapShader->Bind();
		s_CubemapShader->SetUniformBlock("Camera", 0);

		s_LightingShader->Bind();
		s_LightingShader->SetUniformBlock("Camera", 0);
		s_LightingShader->SetUniformBlock("PointLightBuffer", 1);
		s_LightingShader->SetUniformBlock("DirectionalLightBuffer", 2);

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

			s_CubeVertexArray = VertexArray::Create();
			s_CubeVertexArray->AddVertexBuffer(cubeVertexBuffer);
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

			s_QuadVertexArray = VertexArray::Create();

			Ref<VertexBuffer> quadVertexBuffer = VertexBuffer::Create(vertices, 20 * sizeof(float));
			quadVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			});
			s_QuadVertexArray->AddVertexBuffer(quadVertexBuffer);

			Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(indices, 6);
			s_QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
		}
	}

	void Renderer3D::Shutdown()
	{
		ARC_PROFILE_SCOPE();

	}

	void Renderer3D::BeginScene(const CameraData& cameraData, Entity cubemap, std::vector<Entity>&& lights)
	{
		ARC_PROFILE_SCOPE();

		s_Skylight = cubemap;
		s_SceneLights = std::move(lights);

		SetupCameraData(cameraData);
		SetupLightsData();
	}

	void Renderer3D::EndScene(const Ref<RenderGraphData>& renderTarget)
	{
		ARC_PROFILE_SCOPE();

		Flush(renderTarget);

		s_Shader->Unbind();
	}

	void Renderer3D::DrawCube()
	{
		ARC_PROFILE_SCOPE();

		RenderCommand::Draw(s_CubeVertexArray, 36);
	}

	void Renderer3D::DrawQuad()
	{
		ARC_PROFILE_SCOPE();

		RenderCommand::DrawIndexed(s_QuadVertexArray);
	}

	void Renderer3D::ReserveMeshes(const size_t count)
	{
		s_Meshes.reserve(count);
	}

	void Renderer3D::SubmitMesh(const glm::mat4& transform, const Submesh& submesh, MeshComponent::CullModeType cullMode)
	{
		ARC_PROFILE_SCOPE();

		s_Meshes.emplace_back(transform, submesh, cullMode);
	}

	void Renderer3D::Flush(const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();
		
		ShadowMapPass();
		RenderPass(renderGraphData->RenderPassTarget);
		LightingPass(renderGraphData);
		BloomPass(renderGraphData);
		FXAAPass(renderGraphData);
		CompositePass(renderGraphData);
		s_Meshes.clear();
	}

	void Renderer3D::FXAAPass(const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		if (UseFXAA)
		{
			renderGraphData->FXAAPassTarget->Bind();
			s_FxaaShader->Bind();
			s_FxaaShader->SetFloat2("u_Threshold", FXAAThreshold);
			s_FxaaShader->SetInt("u_Texture", 0);
			renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);
			DrawQuad();
		}
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

	void Renderer3D::SetupCameraData(const CameraData& cameraData)
	{
		ARC_PROFILE_SCOPE();

		static_assert(sizeof(CameraData) == sizeof(glm::mat4) * 3 + sizeof(glm::vec3));
		s_UbCamera->Bind();
		s_UbCamera->SetData(&cameraData, 0, sizeof(CameraData));
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
			constexpr uint32_t size = sizeof(PointLightData);

			s_UbPointLights->Bind();

			for (Entity e : s_SceneLights)
			{
				const LightComponent& lightComponent = e.GetComponent<LightComponent>();
				if (lightComponent.Type == LightComponent::LightType::Directional)
					continue;
				glm::mat4 worldTransform = e.GetWorldTransform();
			
				glm::vec4 attenFactors = glm::vec4(
					lightComponent.Range,
					glm::cos(lightComponent.CutOffAngle),
					glm::cos(lightComponent.OuterCutOffAngle),
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

				s_UbPointLights->SetData((void*)(&pointLightData), size * numLights, size);

				numLights++;
			}

			// Pass number of lights within the scene
			s_UbPointLights->SetData(&numLights, MAX_NUM_LIGHTS * size, sizeof(uint32_t));
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
			constexpr uint32_t size = sizeof(DirectionalLightData);

			s_UbDirectionalLights->Bind();

			for (Entity e : s_SceneLights)
			{
				const LightComponent& lightComponent = e.GetComponent<LightComponent>();
				if (lightComponent.Type != LightComponent::LightType::Directional)
					continue;

				glm::mat4 worldTransform = e.GetWorldTransform();
			
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

				s_UbDirectionalLights->SetData((void*)(&dirLightData), size * numLights, size);

				numLights++;
			}

			// Pass number of lights within the scene
			s_UbDirectionalLights->SetData(&numLights, MAX_NUM_DIR_LIGHTS * size, sizeof(uint32_t));
		}
	}

	void Renderer3D::CompositePass(const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		renderGraphData->CompositePassTarget->Bind();
		s_HdrShader->Bind();
		glm::vec4 tonemappingParams = glm::vec4(((int) Tonemapping), Exposure, 0.0f, 0.0f);
		
		s_HdrShader->SetFloat4("u_TonemappParams", tonemappingParams);
		s_HdrShader->SetFloat("u_BloomStrength", UseBloom ? BloomStrength : 0.0f);
		s_HdrShader->SetInt("u_Texture", 0);
		s_HdrShader->SetInt("u_BloomTexture", 1);
		s_HdrShader->SetInt("u_VignetteMask", 2);
		
		if (UseFXAA)
			renderGraphData->FXAAPassTarget->BindColorAttachment(0, 0);
		else
			renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);
		
		renderGraphData->UpsampledFramebuffers[0]->BindColorAttachment(0, 1);

		if (VignetteOffset.a > 0.0f)
		{
			s_HdrShader->SetFloat4("u_VignetteColor", VignetteColor);
			if (VignetteMask)
				VignetteMask->Bind(2);
		}
		s_HdrShader->SetFloat4("u_VignetteOffset", VignetteOffset);

		DrawQuad();
	}

	void Renderer3D::BloomPass(const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		if (!UseBloom)
			return;
		
		glm::vec4 threshold = glm::vec4(BloomThreshold, BloomKnee, BloomKnee * 2.0f, 0.25f / BloomKnee);
		glm::vec4 params = glm::vec4(BloomClamp, 2.0f, 0.0f, 0.0f);

		{
			ARC_PROFILE_SCOPE("Prefilter");

			renderGraphData->PrefilteredFramebuffer->Bind();
			s_BloomShader->Bind();
			s_BloomShader->SetFloat4("u_Threshold", threshold);
			s_BloomShader->SetFloat4("u_Params", params);
			s_BloomShader->SetInt("u_Texture", 0);
			renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);
			DrawQuad();
		}

		size_t blurSamples = renderGraphData->BlurSamples;
		FramebufferSpecification spec = renderGraphData->PrefilteredFramebuffer->GetSpecification();
		{
			ARC_PROFILE_SCOPE("Downsample");

			s_GaussianBlurShader->Bind();
			s_GaussianBlurShader->SetInt("u_Texture", 0);
			for (size_t i = 0; i < blurSamples; i++)
			{
				renderGraphData->TempBlurFramebuffers[i]->Bind();
				s_GaussianBlurShader->SetInt("u_Horizontal", static_cast<int>(true));
				if (i == 0)
					renderGraphData->PrefilteredFramebuffer->BindColorAttachment(0, 0);
				else
					renderGraphData->DownsampledFramebuffers[i - 1]->BindColorAttachment(0, 0);
				DrawQuad();

				renderGraphData->DownsampledFramebuffers[i]->Bind();
				s_GaussianBlurShader->SetInt("u_Horizontal", static_cast<int>(false));
				renderGraphData->TempBlurFramebuffers[i]->BindColorAttachment(0, 0);
				DrawQuad();
			}
		}
		
		{
			ARC_PROFILE_SCOPE("Upsample");

			s_BloomShader->Bind();
			s_BloomShader->SetFloat4("u_Threshold", threshold);
			params = glm::vec4(BloomClamp, 3.0f, 1.0f, 1.0f);
			s_BloomShader->SetFloat4("u_Params", params);
			s_BloomShader->SetInt("u_Texture", 0);
			s_BloomShader->SetInt("u_AdditiveTexture", 1);
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
			s_BloomShader->SetFloat4("u_Params", params);
			renderGraphData->UpsampledFramebuffers[1]->BindColorAttachment(0, 0);
			DrawQuad();
		}
	}

	void Renderer3D::LightingPass(const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		renderGraphData->LightingPassTarget->Bind();
		RenderCommand::Clear();

		s_LightingShader->Bind();

		s_LightingShader->SetInt("u_Albedo", 0);
		s_LightingShader->SetInt("u_Normal", 1);
		s_LightingShader->SetInt("u_MetallicRoughnessAO", 2);
		s_LightingShader->SetInt("u_Emission", 3);
		s_LightingShader->SetInt("u_Depth", 4);

		s_LightingShader->SetInt("u_IrradianceMap", 5);
		s_LightingShader->SetInt("u_RadianceMap", 6);
		s_LightingShader->SetInt("u_BRDFLutMap", 7);

		int32_t samplers[MAX_NUM_DIR_LIGHTS];
		for (uint32_t i = 0; i < MAX_NUM_DIR_LIGHTS; i++)
			samplers[i] = i + 8;
		s_LightingShader->SetIntArray("u_DirectionalShadowMap", samplers, MAX_NUM_DIR_LIGHTS);

		renderGraphData->RenderPassTarget->BindColorAttachment(0, 0);
		renderGraphData->RenderPassTarget->BindColorAttachment(1, 1);
		renderGraphData->RenderPassTarget->BindColorAttachment(2, 2);
		renderGraphData->RenderPassTarget->BindColorAttachment(3, 3);
		renderGraphData->RenderPassTarget->BindDepthAttachment(4);
		
		if (s_Skylight)
		{
			const SkyLightComponent& skylightComponent = s_Skylight.GetComponent<SkyLightComponent>();
			if (skylightComponent.Texture)
			{
				s_LightingShader->SetFloat("u_IrradianceIntensity", skylightComponent.Intensity);
				s_LightingShader->SetFloat("u_EnvironmentRotation", skylightComponent.Rotation);
				skylightComponent.Texture->BindIrradianceMap(5);
				skylightComponent.Texture->BindRadianceMap(6);
			}
		}
		s_BRDFLutTexture->Bind(7);
		
		uint32_t dirLightIndex = 0;
		for (const auto& lightEntity : s_SceneLights)
		{
			const LightComponent& light = lightEntity.GetComponent<LightComponent>();
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

	void Renderer3D::RenderPass(const Ref<Framebuffer>& renderTarget)
	{
		ARC_PROFILE_SCOPE();

		renderTarget->Bind();
		RenderCommand::SetBlendState(false);
		RenderCommand::SetClearColor(glm::vec4(0.0f));
		RenderCommand::Clear();

		const SkyLightComponent* skylightComponent = nullptr;
		float skylightIntensity = 0.0f;
		float skylightRotation = 0.0f;
		{
			ARC_PROFILE_SCOPE("Skylight");

			if (s_Skylight)
			{
				skylightComponent = &(s_Skylight.GetComponent<SkyLightComponent>());
				if (skylightComponent->Texture)
				{
					RenderCommand::SetDepthMask(false);

					skylightIntensity = skylightComponent->Intensity;
					skylightRotation = skylightComponent->Rotation;

					skylightComponent->Texture->Bind(0);
					s_CubemapShader->Bind();
					s_CubemapShader->SetFloat("u_Intensity", skylightIntensity);
					s_CubemapShader->SetFloat("u_Rotation", skylightRotation);

					DrawCube();

					RenderCommand::SetDepthMask(true);
				}
			}
		}
		
		{
			ARC_PROFILE_SCOPE("Draw Meshes");

			MeshComponent::CullModeType currentCullMode = MeshComponent::CullModeType::Unknown;
			for (const auto& meshData : s_Meshes)
			{
				meshData.SubmeshGeometry.Mat->Bind();
				s_Shader->SetMat4("u_Model", meshData.Transform);
				
				if (currentCullMode != meshData.CullMode)
				{
					currentCullMode = meshData.CullMode;
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
							break;
					}
				}
				
				RenderCommand::DrawIndexed(meshData.SubmeshGeometry.Geometry);
				s_Stats.DrawCalls++;
				s_Stats.IndexCount += meshData.SubmeshGeometry.Geometry->GetIndexBuffer()->GetCount();
			}
		}
	}

	void Renderer3D::ShadowMapPass()
	{
		ARC_PROFILE_SCOPE();

		for (const auto& lightEntity : s_SceneLights)
		{
			const LightComponent& light = lightEntity.GetComponent<LightComponent>();
			if (light.Type != LightComponent::LightType::Directional)
				continue;

			light.ShadowMapFramebuffer->Bind();
			RenderCommand::Clear();

			glm::mat4 transform = lightEntity.GetWorldTransform();
			float near_plane = -100.0f, far_plane = 100.0f;
			
			glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
			
			glm::vec4 zDir = transform * glm::vec4(0, 0, 1, 0);

			glm::vec3 pos = transform[3];
			glm::vec3 dir = glm::normalize(glm::vec3(zDir));
			glm::vec3 lookAt = pos + dir;
			glm::mat4 dirLightView = glm::lookAt(pos, lookAt, glm::vec3(0, 1 ,0));
			glm::mat4 dirLightViewProj = lightProjection * dirLightView;

			s_ShadowMapShader->Bind();
			s_ShadowMapShader->SetMat4("u_ViewProjection", dirLightViewProj);

			for (auto it = s_Meshes.rbegin(); it != s_Meshes.rend(); it++)
			{
				const MeshData* meshData = &(*it);
				s_ShadowMapShader->SetMat4("u_Model", meshData->Transform);
				RenderCommand::DrawIndexed(meshData->SubmeshGeometry.Geometry);
			}
		}
	}
}
