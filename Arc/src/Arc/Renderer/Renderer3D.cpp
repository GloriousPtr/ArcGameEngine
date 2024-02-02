#include "arcpch.h"
#include "Renderer3D.h"

#include "Renderer.h"
#include "RenderGraphData.h"
#include "Framebuffer.h"
#include "Arc/Renderer/VertexArray.h"
#include "Arc/Renderer/Material.h"
#include "Arc/Renderer/Shader.h"
#include "Arc/Renderer/PipelineState.h"

#include "Arc/Scene/Entity.h"

namespace ArcEngine
{
	struct MeshData
	{
		glm::mat4 Transform;
		Submesh& SubmeshGeometry;

		MeshData(const glm::mat4& transform, Submesh& submesh)
			: Transform(transform), SubmeshGeometry(submesh)
		{
		}
	};

	struct SkyboxData
	{
		glm::mat4 SkyboxViewProjection;
		glm::vec2 RotationIntensity;
	};

	struct DirectionalLightData
	{
		glm::vec4 Direction;
		glm::vec4 Color;				// rgb: color, a: intensity
	};

	struct PointLightData
	{
		glm::vec4 Position;				// xyz: position, w: radius
		glm::vec4 Color;				// rgb: color, a: intensity
	};

	struct SpotLightData
	{
		glm::vec4 Position;				// xyz: position, w: radius
		glm::vec4 Color;				// rgb: color, a: intensity
		glm::vec4 AttenuationFactors;	// xy: cut-off angles
		glm::vec4 Direction;
	};

	struct Renderer3DData
	{
		Renderer3D::Statistics Stats;
		eastl::vector<MeshData> Meshes;
		Ref<Texture2D> BRDFLutTexture;
		Ref<PipelineState> RenderPipeline;
		Ref<PipelineState> CubemapPipeline;
		Ref<PipelineState> LightingPipeline;
		GraphicsCommandList CommandList;

		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<VertexArray> QuadVertexArray;

		Entity Skylight;
		eastl::vector<Entity> SceneLights;
		GlobalData GlobalData;
	};

	static Scope<Renderer3DData> s_Renderer3DData;

#if 0
	Ref<PipelineState> Renderer3D::s_LightingShader;
	Ref<PipelineState> Renderer3D::s_ShadowMapShader;
	Ref<PipelineState> Renderer3D::s_CubemapShader;
	Ref<PipelineState> Renderer3D::s_GaussianBlurShader;
	Ref<PipelineState> Renderer3D::s_FxaaShader;
	Ref<PipelineState> Renderer3D::s_HdrShader;
	Ref<PipelineState> Renderer3D::s_BloomShader;
	Ref<VertexBuffer> Renderer3D::s_CubeVertexBuffer;

	Ref<Texture2D> Renderer3D::s_BRDFLutTexture;

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
#endif

	void Renderer3D::Init()
	{
		ARC_PROFILE_SCOPE();

		s_Renderer3DData = CreateScope<Renderer3DData>();

		PipelineLibrary& pipelineLibrary = Renderer::GetPipelineLibrary();

		{
			PipelineSpecification spec
			{
				.Type = ShaderType::Pixel,
				.GraphicsPipelineSpecs
				{
					.CullMode = CullModeType::Back,
					.Primitive = PrimitiveType::Triangle,
					.FillMode = FillModeType::Solid,
					.DepthFunc = DepthFuncType::Less,
					.DepthFormat = FramebufferTextureFormat::Depth,
					.OutputFormats = {	FramebufferTextureFormat::RGBA8,				// Albedo
										FramebufferTextureFormat::RGBA16F,				// Normal
										FramebufferTextureFormat::RGBA8,				// Metallic, Roughness, AO
										FramebufferTextureFormat::RGBA8,				// rgb: EmissionColor, a: intensity
									 }
				}
			};
			s_Renderer3DData->RenderPipeline = pipelineLibrary.Load("assets/shaders/PBR.hlsl", spec);
		}

		{
			PipelineSpecification spec
			{ 
				.Type = ShaderType::Pixel,
				.GraphicsPipelineSpecs
				{
					.CullMode = CullModeType::Back,
					.Primitive = PrimitiveType::Triangle,
					.FillMode = FillModeType::Solid,
					.DepthFormat = FramebufferTextureFormat::None,
					.OutputFormats = { FramebufferTextureFormat::R11G11B10F }
				}
			};
			s_Renderer3DData->LightingPipeline = pipelineLibrary.Load("assets/shaders/LightingPass.hlsl", spec);
		}

		{
			PipelineSpecification spec
			{
				.Type = ShaderType::Pixel,
				.GraphicsPipelineSpecs
				{
					.CullMode = CullModeType::Front,
					.Primitive = PrimitiveType::Triangle,
					.FillMode = FillModeType::Solid,
					.DepthFormat = FramebufferTextureFormat::None,
					.OutputFormats = {FramebufferTextureFormat::RGBA8}
				}
			};
			s_Renderer3DData->CubemapPipeline = pipelineLibrary.Load("assets/shaders/Cubemap.hlsl", spec);
		}

		s_Renderer3DData->RenderPipeline->RegisterCB("GlobalData", sizeof(GlobalData));
		s_Renderer3DData->LightingPipeline->RegisterCB("GlobalData", sizeof(GlobalData));
		s_Renderer3DData->CubemapPipeline->RegisterCB("SkyboxData", sizeof(SkyboxData));
		s_Renderer3DData->LightingPipeline->RegisterSB("DirectionalLights", sizeof(DirectionalLightData), MAX_NUM_DIR_LIGHTS);
		s_Renderer3DData->LightingPipeline->RegisterSB("PointLights", sizeof(PointLightData), MAX_NUM_POINT_LIGHTS);
		s_Renderer3DData->LightingPipeline->RegisterSB("SpotLights", sizeof(SpotLightData), MAX_NUM_SPOT_LIGHTS);

#if 0
		s_BRDFLutTexture = Texture2D::Create("Resources/Renderer/BRDF_LUT.jpg");

		s_ShadowMapShader = pipelineLibrary.Load("assets/shaders/DepthShader.glsl", spec);
		s_CubemapShader = pipelineLibrary.Load("assets/shaders/Cubemap.glsl", spec);
		s_GaussianBlurShader = pipelineLibrary.Load("assets/shaders/GaussianBlur.glsl", spec);
		s_FxaaShader = pipelineLibrary.Load("assets/shaders/FXAA.glsl", spec);
		s_HdrShader = pipelineLibrary.Load("assets/shaders/HDR.glsl", spec);
		s_BloomShader = pipelineLibrary.Load("assets/shaders/Bloom.glsl", spec);
		s_LightingShader = pipelineLibrary.Load("assets/shaders/LightingPass.glsl", spec);
#endif
		// Cube-map
		{
			constexpr float vertices[108] = {
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

			s_Renderer3DData->CubeVertexBuffer = VertexBuffer::Create(vertices, 108 * sizeof(float), 3 * sizeof(float));
		}

		// Quad
		{
			constexpr float vertices[24] = {
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
				-1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f
			};

			constexpr uint32_t indices[6] = {
				0, 1, 2,
				0, 2, 3
			};

			s_Renderer3DData->QuadVertexArray = VertexArray::Create();
			Ref<VertexBuffer> quadVertexBuffer = VertexBuffer::Create(vertices, 24 * sizeof(float), 6 * sizeof(float));
			s_Renderer3DData->QuadVertexArray->AddVertexBuffer(quadVertexBuffer);
			Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(indices, 6);
			s_Renderer3DData->QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
		}
	}

	void Renderer3D::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		s_Renderer3DData.reset();
	}

	void Renderer3D::BeginScene(const CameraData& cameraData, Entity cubemap, eastl::vector<Entity>&& lights)
	{
		ARC_PROFILE_SCOPE();

		s_Renderer3DData->Skylight = cubemap;
		s_Renderer3DData->SceneLights = eastl::move(lights);
		s_Renderer3DData->GlobalData =
		{
			.CameraView = cameraData.View,
			.CameraProjection = cameraData.Projection,
			.CameraViewProjection = cameraData.ViewProjection,
			.CameraPosition = cameraData.Position,
			.InvCameraView = glm::inverse(cameraData.View),
			.InvCameraProjection = glm::inverse(cameraData.Projection),
			.NumDirectionalLights = 0,
			.NumPointLights = 0,
			.NumSpotLights = 0
		};

		GraphicsCommandList cl = RenderCommand::BeginRecordingCommandList();

		s_Renderer3DData->LightingPipeline->Bind(cl);
		SetupLightsData(cl);
		s_Renderer3DData->LightingPipeline->SetCBData(cl, CRC32("GlobalData"), &(s_Renderer3DData->GlobalData), sizeof(GlobalData));

		s_Renderer3DData->RenderPipeline->Bind(cl);
		s_Renderer3DData->RenderPipeline->SetCBData(cl, CRC32("GlobalData"), &(s_Renderer3DData->GlobalData), sizeof(GlobalData));
		
		if (cubemap)
		{
			s_Renderer3DData->CubemapPipeline->Bind(cl);
			SkyLightComponent& sky = cubemap.GetComponent<SkyLightComponent>();
			const SkyboxData data
			{
				.SkyboxViewProjection = cameraData.Projection * glm::mat4(glm::mat3(cameraData.View)),
				.RotationIntensity = { sky.Rotation, sky.Intensity }
			};
			s_Renderer3DData->CubemapPipeline->SetCBData(cl, CRC32("SkyboxData"), &data, sizeof(SkyboxData));
		}
		RenderCommand::EndRecordingCommandList(cl);
	}

	void Renderer3D::EndScene(const Ref<RenderGraphData>& renderTarget)
	{
		ARC_PROFILE_SCOPE();

		Flush(renderTarget);
		s_Renderer3DData->Meshes.clear();
	}

	void Renderer3D::DrawCube(GraphicsCommandList cl)
	{
		ARC_PROFILE_SCOPE();

		RenderCommand::Draw(cl, s_Renderer3DData->CubeVertexBuffer, 36);
	}

	void Renderer3D::DrawQuad(GraphicsCommandList cl)
	{
		ARC_PROFILE_SCOPE();

		RenderCommand::DrawIndexed(cl, s_Renderer3DData->QuadVertexArray);
	}

	void Renderer3D::ReserveMeshes(size_t count)
	{
		s_Renderer3DData->Meshes.reserve(count);
	}

	void Renderer3D::SubmitMesh(const glm::mat4& transform, Submesh& submesh)
	{
		ARC_PROFILE_SCOPE();
		s_Renderer3DData->Meshes.emplace_back(transform, submesh);
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
	}

	void Renderer3D::FXAAPass([[maybe_unused]] const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

#if 0
		if (UseFXAA)
		{
			renderGraphData->FXAAPassTarget->Bind();
			//s_FxaaShader->SetData("u_Threshold", FXAAThreshold);
			//s_FxaaShader->SetData("u_Texture", 0);
			renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);
			DrawQuad();
		}
#endif
	}

	void Renderer3D::ResetStats()
	{
		ARC_PROFILE_SCOPE();

		memset(&s_Renderer3DData->Stats, 0, sizeof(Statistics));
	}

	Renderer3D::Statistics Renderer3D::GetStats()
	{
		ARC_PROFILE_SCOPE();

		return s_Renderer3DData->Stats;
	}

	void Renderer3D::SetupLightsData(GraphicsCommandList cl)
	{
		ARC_PROFILE_SCOPE();

		{
			uint32_t numDirectionalLights = 0;
			uint32_t numPointLights = 0;
			uint32_t numSpotLights = 0;

			eastl::array<DirectionalLightData, MAX_NUM_DIR_LIGHTS> dlData{};
			eastl::array<PointLightData, MAX_NUM_POINT_LIGHTS> plData{};
			eastl::array<SpotLightData, MAX_NUM_SPOT_LIGHTS> slData{};

			for (Entity e : s_Renderer3DData->SceneLights)
			{
				const LightComponent& lightComponent = e.GetComponent<LightComponent>();
				glm::mat4 worldTransform = e.GetWorldTransform();

				switch (lightComponent.Type)
				{
					case LightComponent::LightType::Directional:
						{
							dlData[numDirectionalLights] =
							{
								.Direction = worldTransform * glm::vec4(0, 0, 1, 0),
								.Color = glm::vec4(lightComponent.Color, lightComponent.Intensity)
							};
							++numDirectionalLights;
						}
						break;
					case LightComponent::LightType::Point:
						{
							plData[numPointLights] = PointLightData
							{
								.Position = glm::vec4(glm::vec3(worldTransform[3]), lightComponent.Range),
								.Color = glm::vec4(lightComponent.Color, lightComponent.Intensity),
							};
							++numPointLights;
						}
						break;
					case LightComponent::LightType::Spot:
						{
							slData[numSpotLights] = SpotLightData
							{
								.Position = glm::vec4(glm::vec3(worldTransform[3]), lightComponent.Range),
								.Color = glm::vec4(lightComponent.Color, lightComponent.Intensity),
								.AttenuationFactors = glm::vec4(glm::cos(lightComponent.CutOffAngle), glm::cos(lightComponent.OuterCutOffAngle), 0.0f, 0.0f),
								.Direction = worldTransform * glm::vec4(0, 0, 1, 0)
							};
							++numSpotLights;
						}
						break;
					default: ;
				}
			}

			s_Renderer3DData->GlobalData.NumDirectionalLights = numDirectionalLights;
			s_Renderer3DData->GlobalData.NumPointLights = numPointLights;
			s_Renderer3DData->GlobalData.NumSpotLights = numSpotLights;

			s_Renderer3DData->LightingPipeline->SetSBData(cl, CRC32("DirectionalLights"), dlData.data(), sizeof(DirectionalLightData) * numDirectionalLights, 0);
			s_Renderer3DData->LightingPipeline->SetSBData(cl, CRC32("PointLights"), plData.data(), sizeof(PointLightData) * numPointLights, 0);
			s_Renderer3DData->LightingPipeline->SetSBData(cl, CRC32("SpotLights"), slData.data(), sizeof(SpotLightData) * numSpotLights, 0);
		}
#if 0
		{
			uint32_t numLights = 0;
			constexpr uint32_t size = sizeof(DirectionalLightData);

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
					glm::vec4(pos, static_cast<uint32_t>(lightComponent.ShadowQuality)),
					glm::vec4(lightComponent.Color, lightComponent.Intensity),
					zDir,
					dirLightViewProj
				};

				s_UbPointLights->Bind(numLights);
				s_UbDirectionalLights->SetData(&dirLightData, size, numLights);

				numLights++;
			}

			s_NumDirectionalLights = numLights;
		}
#endif
	}

	void Renderer3D::CompositePass([[maybe_unused]] const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

#if 0
		renderGraphData->CompositePassTarget->Bind();
		const glm::vec4 tonemappingParams = { static_cast<int>(Tonemapping), Exposure, 0.0f, 0.0f };

		/*
		s_HdrShader->SetData("u_TonemappParams", tonemappingParams);
		s_HdrShader->SetData("u_BloomStrength", UseBloom ? BloomStrength : -1.0f);
		s_HdrShader->SetData("u_Texture", 0);
		s_HdrShader->SetData("u_BloomTexture", 1);
		s_HdrShader->SetData("u_VignetteMask", 2);
		*/

		if (UseFXAA)
			renderGraphData->FXAAPassTarget->BindColorAttachment(0, 0);
		else
			renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);

		if (UseBloom)
			renderGraphData->UpsampledFramebuffers[0]->BindColorAttachment(0, 1);

		/* 
		if (VignetteOffset.a > 0.0f)
		{
			s_HdrShader->SetData("u_VignetteColor", VignetteColor);
			if (VignetteMask)
				VignetteMask->Bind(2);
		}
		s_HdrShader->SetData("u_VignetteOffset", VignetteOffset);
		*/

		DrawQuad();
#endif
	}

	void Renderer3D::BloomPass([[maybe_unused]] const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

#if 0
		if (!UseBloom)
			return;

		const glm::vec4 threshold = { BloomThreshold, BloomKnee, BloomKnee * 2.0f, 0.25f / BloomKnee };
		glm::vec4 params = { BloomClamp, 2.0f, 0.0f, 0.0f };

		{
			ARC_PROFILE_SCOPE_NAME("Prefilter");

			renderGraphData->PrefilteredFramebuffer->Bind();
			//s_BloomShader->SetData("u_Threshold", threshold);
			//s_BloomShader->SetData("u_Params", params);
			//s_BloomShader->SetData("u_Texture", 0);
			renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);
			DrawQuad();
		}

		const size_t blurSamples = renderGraphData->BlurSamples;
		FramebufferSpecification spec = renderGraphData->PrefilteredFramebuffer->GetSpecification();
		{
			ARC_PROFILE_SCOPE_NAME("Downsample");

			//s_GaussianBlurShader->SetData("u_Texture", 0);
			for (size_t i = 0; i < blurSamples; i++)
			{
				renderGraphData->TempBlurFramebuffers[i]->Bind();
				//s_GaussianBlurShader->SetData("u_Horizontal", static_cast<int>(true));
				if (i == 0)
					renderGraphData->PrefilteredFramebuffer->BindColorAttachment(0, 0);
				else
					renderGraphData->DownsampledFramebuffers[i - 1]->BindColorAttachment(0, 0);
				DrawQuad();

				renderGraphData->DownsampledFramebuffers[i]->Bind();
				//s_GaussianBlurShader->SetData("u_Horizontal", static_cast<int>(false));
				renderGraphData->TempBlurFramebuffers[i]->BindColorAttachment(0, 0);
				DrawQuad();
			}
		}
		
		{
			ARC_PROFILE_SCOPE_NAME("Upsample");

			//s_BloomShader->SetData("u_Threshold", threshold);
			params = glm::vec4(BloomClamp, 3.0f, 1.0f, 1.0f);
			//s_BloomShader->SetData("u_Params", params);
			//s_BloomShader->SetData("u_Texture", 0);
			//s_BloomShader->SetData("u_AdditiveTexture", 1);
			const size_t upsampleCount = blurSamples - 1;
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
			//s_BloomShader->SetData("u_Params", params);
			renderGraphData->UpsampledFramebuffers[1]->BindColorAttachment(0, 0);
			DrawQuad();
		}
#endif
	}

	void Renderer3D::LightingPass(const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		GraphicsCommandList cl = RenderCommand::BeginRecordingCommandList();
		renderGraphData->LightingPassTarget->Clear(cl);
		renderGraphData->LightingPassTarget->Bind(cl);
		s_Renderer3DData->LightingPipeline->Bind(cl);
		s_Renderer3DData->LightingPipeline->BindCB(cl, CRC32("GlobalData"));
		s_Renderer3DData->LightingPipeline->BindSB(cl, CRC32("DirectionalLights"));
		s_Renderer3DData->LightingPipeline->BindSB(cl, CRC32("PointLights"));
		s_Renderer3DData->LightingPipeline->BindSB(cl, CRC32("SpotLights"));
		renderGraphData->RenderPassTarget->BindColorAttachment(cl, 0, s_Renderer3DData->LightingPipeline->GetSlot("Albedo"));
		renderGraphData->RenderPassTarget->BindColorAttachment(cl, 1, s_Renderer3DData->LightingPipeline->GetSlot("Normal"));
		renderGraphData->RenderPassTarget->BindColorAttachment(cl, 2, s_Renderer3DData->LightingPipeline->GetSlot("MetalicRoughnessAO"));
		renderGraphData->RenderPassTarget->BindColorAttachment(cl, 3, s_Renderer3DData->LightingPipeline->GetSlot("Emission"));
		renderGraphData->RenderPassTarget->BindDepthAttachment(cl, s_Renderer3DData->LightingPipeline->GetSlot("Depth"));
		DrawQuad(cl);
		renderGraphData->LightingPassTarget->Unbind(cl);
		RenderCommand::EndRecordingCommandList(cl);

#if 0
		renderGraphData->LightingPassTarget->Bind();
		RenderCommand::Clear();


		//s_LightingShader->SetData("u_NumPointLights", static_cast<int>(s_NumPointLights));
		//s_LightingShader->SetData("u_NumDirectionalLights", static_cast<int>(s_NumDirectionalLights));

		//s_LightingShader->SetData("u_Albedo", 0);
		//s_LightingShader->SetData("u_Normal", 1);
		//s_LightingShader->SetData("u_MetallicRoughnessAO", 2);
		//s_LightingShader->SetData("u_Emission", 3);
		//s_LightingShader->SetData("u_Depth", 4);

		//s_LightingShader->SetData("u_IrradianceMap", 5);
		//s_LightingShader->SetData("u_RadianceMap", 6);
		//s_LightingShader->SetData("u_BRDFLutMap", 7);

		int32_t samplers[MAX_NUM_DIR_LIGHTS];
		for (uint32_t i = 0; i < MAX_NUM_DIR_LIGHTS; i++)
			samplers[i] = static_cast<int32_t>(i + 8);
		//s_LightingShader->SetData("u_DirectionalShadowMap", samplers, sizeof(int32_t) * MAX_NUM_DIR_LIGHTS);

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
				//s_LightingShader->SetData("u_IrradianceIntensity", skylightComponent.Intensity);
				//s_LightingShader->SetData("u_EnvironmentRotation", skylightComponent.Rotation);
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
#endif
	}

	void Renderer3D::RenderPass(const Ref<Framebuffer>& renderTarget)
	{
		ARC_PROFILE_SCOPE();

		GraphicsCommandList cl = RenderCommand::BeginRecordingCommandList();
		renderTarget->Clear(cl);
		renderTarget->Bind(cl);

		if (s_Renderer3DData->Skylight)
		{
			ARC_PROFILE_SCOPE_NAME("Draw Skylight");

			s_Renderer3DData->CubemapPipeline->Bind(cl);
			const SkyLightComponent& skylightComponent = s_Renderer3DData->Skylight.GetComponent<SkyLightComponent>();
			if (skylightComponent.Texture)
			{
				s_Renderer3DData->CubemapPipeline->BindCB(cl, CRC32("SkyboxData"));
				skylightComponent.Texture->Bind(cl, s_Renderer3DData->CubemapPipeline->GetSlot("EnvironmentTexture"));
				DrawCube(cl);
			}
		}

		if (!s_Renderer3DData->Meshes.empty())
		{
			ARC_PROFILE_SCOPE_NAME("Draw Meshes");

			s_Renderer3DData->RenderPipeline->Bind(cl);
			s_Renderer3DData->RenderPipeline->BindCB(cl, CRC32("GlobalData"));
			for (const MeshData& meshData : s_Renderer3DData->Meshes)
			{
				meshData.SubmeshGeometry.Mat->Bind(cl);
				s_Renderer3DData->RenderPipeline->SetRSData(cl, "Transform", glm::value_ptr(meshData.Transform), sizeof(glm::mat4));
				RenderCommand::DrawIndexed(cl, meshData.SubmeshGeometry.Geometry);
				s_Renderer3DData->Stats.DrawCalls++;
				s_Renderer3DData->Stats.IndexCount += meshData.SubmeshGeometry.Geometry->GetIndexBuffer()->GetCount();
			}
		}

		renderTarget->Unbind(cl);
		RenderCommand::EndRecordingCommandList(cl);

#if 0
		renderTarget->Bind();
		//RenderCommand::SetBlendState(false);
		RenderCommand::SetClearColor(glm::vec4(0.0f));
		RenderCommand::Clear();

		{
			ARC_PROFILE_SCOPE_NAME("Skylight");

			if (s_Skylight)
			{
				const SkylightComponent& skylightComponent = s_Skylight.GetComponent<SkyLightComponent>();
				if (skylightComponent.Texture)
				{
					//RenderCommand::SetDepthMask(false);

					skylightComponent.Texture->Bind(0);
					//s_CubemapShader->SetData("u_Intensity", skylightComponent.Intensity);
					//s_CubemapShader->SetData("u_Rotation", skylightComponent.Rotation);

					DrawCube();

					//RenderCommand::SetDepthMask(true);
				}
			}
		}
		
		{
			ARC_PROFILE_SCOPE_NAME("Draw Meshes");

			MeshComponent::CullModeType currentCullMode = MeshComponent::CullModeType::Unknown;
			const std::span<MeshData> meshes(s_Renderer3DData->Meshes.data(), s_Renderer3DData->MeshInsertIndex);
			for (const MeshData& meshData : meshes)
			{
				meshData.Mat->Bind();
				//s_Shader->SetData("u_Model", meshData.Transform);
				
				RenderCommand::DrawIndexed(meshData.Geometry);
				s_Stats.DrawCalls++;
				s_Stats.IndexCount += meshData.Geometry->GetIndexBuffer()->GetCount();
			}
		}
#endif
	}

	void Renderer3D::ShadowMapPass()
	{
		ARC_PROFILE_SCOPE();
#if 0
		for (const auto& lightEntity : s_SceneLights)
		{
			const LightComponent& light = lightEntity.GetComponent<LightComponent>();
			if (light.Type != LightComponent::LightType::Directional)
				continue;

			light.ShadowMapFramebuffer->Bind();
			RenderCommand::Clear();

			glm::mat4 transform = lightEntity.GetWorldTransform();
			constexpr float nearPlane = -100.0f;
			constexpr float farPlane = 100.0f;
			
			glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, nearPlane, farPlane);
			
			glm::vec4 zDir = transform * glm::vec4(0, 0, 1, 0);

			glm::vec3 pos = transform[3];
			glm::vec3 dir = glm::normalize(glm::vec3(zDir));
			glm::vec3 lookAt = pos + dir;
			glm::mat4 dirLightView = glm::lookAt(pos, lookAt, glm::vec3(0, 1 ,0));
			glm::mat4 dirLightViewProj = lightProjection * dirLightView;

			//s_ShadowMapShader->SetData("u_ViewProjection", dirLightViewProj);

			const std::span<MeshData> meshes(s_Renderer3DData->Meshes.data(), s_Renderer3DData->MeshInsertIndex);
			for (std::span<MeshData>::iterator it = meshes.rbegin(); it != meshes.rend(); ++it)
			{
				const MeshData& meshData = *it;
				//s_ShadowMapShader->SetData("u_Model", meshData.Transform);
				RenderCommand::DrawIndexed(meshData.Geometry);
			}
		}
#endif
	}
}
