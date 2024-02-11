#include "arcpch.h"
#include "Renderer3D.h"

#include "Renderer.h"
#include "RenderGraphData.h"
#include "Framebuffer.h"
#include "Arc/Core/AssetManager.h"
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

	struct DepthShaderProperties
	{
		glm::mat4 ViewProjection;
	};

	struct LightingShaderProperties
	{
		uint32_t AlbedoTexture;
		uint32_t NormalTexture;
		uint32_t MRATexture;
		uint32_t EmissiveTexture;
		uint32_t DepthTexture;
		uint32_t IrradianceTexture;

		float SkyboxIntensity;
		float SkyboxRotation;
	};

	struct CompositeShaderProperties
	{
		glm::vec4 VignetteColor; // rgb: color, a: intensity
		glm::vec4 VignetteOffset; // xy: offset, z: useMask, w: enable/disable effect
		float TonemapExposure;
		uint32_t TonemapType; // 0 None/ExposureBased, 1: ACES, 2: Filmic, 3: Uncharted

		uint32_t MainTexture;
		uint32_t VignetteMask;
	};

	struct DirectionalLightData
	{
		glm::vec4 Direction;
		glm::vec4 Color;				// rgb: color, a: intensity
		glm::mat4 ViewProjection;
		glm::vec4 QualityTextureBias;
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
		Ref<PipelineState> DepthPipeline;
		Ref<PipelineState> RenderPipeline;
		Ref<PipelineState> CubemapPipeline;
		Ref<PipelineState> LightingPipeline;
		Ref<PipelineState> CompositePipeline;
		GraphicsCommandList CommandList;

		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<VertexArray> QuadVertexArray;

		Entity Skylight;
		eastl::vector<Entity> SceneLights;
		GlobalData GlobalData;
	};

	static Scope<Renderer3DData> s_Renderer3DData;

	Renderer3D::TonemappingType Renderer3D::Tonemapping = Renderer3D::TonemappingType::ACES;
	float Renderer3D::Exposure = 1.0f;
	glm::vec4 Renderer3D::VignetteColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.25f);	// rgb: color, a: intensity
	glm::vec4 Renderer3D::VignetteOffset = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);	// xy: offset, z: useMask, w: enable/disable effect
	Ref<Texture2D> Renderer3D::VignetteMask = nullptr;
#if 0
	Ref<PipelineState> Renderer3D::s_GaussianBlurShader;
	Ref<PipelineState> Renderer3D::s_FxaaShader;
	Ref<PipelineState> Renderer3D::s_HdrShader;
	Ref<PipelineState> Renderer3D::s_BloomShader;

	Ref<Texture2D> Renderer3D::s_BRDFLutTexture;

	bool Renderer3D::UseBloom = true;
	float Renderer3D::BloomStrength = 0.1f;
	float Renderer3D::BloomThreshold = 1.0f;
	float Renderer3D::BloomKnee = 0.1f;
	float Renderer3D::BloomClamp = 100.0f;
	bool Renderer3D::UseFXAA = true;
	glm::vec2 Renderer3D::FXAAThreshold = glm::vec2(0.0078125f, 0.125f);		// x: current threshold, y: relative threshold
#endif

	void Renderer3D::Init()
	{
		ARC_PROFILE_SCOPE();

		s_Renderer3DData = CreateScope<Renderer3DData>();

		PipelineLibrary& pipelineLibrary = Renderer::GetPipelineLibrary();

		s_Renderer3DData->DepthPipeline = pipelineLibrary.Load("assets/shaders/DepthShader.hlsl",
		{
			.Type = ShaderType::Vertex,
			.GraphicsPipelineSpecs
			{
				.CullMode = CullModeType::Back,
				.Primitive = PrimitiveType::Triangle,
				.FillMode = FillModeType::Solid,
				.DepthFunc = DepthFuncType::Less,
				.DepthFormat = FramebufferTextureFormat::Depth
			}
		});
		s_Renderer3DData->RenderPipeline = pipelineLibrary.Load("assets/shaders/PBR.hlsl",
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
		});
		s_Renderer3DData->LightingPipeline = pipelineLibrary.Load("assets/shaders/LightingPass.hlsl",
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
		});
		s_Renderer3DData->CompositePipeline = pipelineLibrary.Load("assets/shaders/CompositePass.hlsl",
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
		});
		s_Renderer3DData->CubemapPipeline = pipelineLibrary.Load("assets/shaders/Cubemap.hlsl",
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
		});

		s_Renderer3DData->DepthPipeline->RegisterCB(CRC32("Properties"), sizeof(DepthShaderProperties));
		s_Renderer3DData->RenderPipeline->RegisterCB(CRC32("GlobalData"), sizeof(GlobalData));
		s_Renderer3DData->LightingPipeline->RegisterCB(CRC32("GlobalData"), sizeof(GlobalData));
		s_Renderer3DData->LightingPipeline->RegisterCB(CRC32("Properties"), sizeof(LightingShaderProperties));
		s_Renderer3DData->CompositePipeline->RegisterCB(CRC32("Properties"), sizeof(CompositeShaderProperties));
		s_Renderer3DData->CubemapPipeline->RegisterCB(CRC32("SkyboxData"), sizeof(SkyboxData));
		s_Renderer3DData->LightingPipeline->RegisterSB(CRC32("DirectionalLights"), sizeof(DirectionalLightData), MAX_NUM_DIR_LIGHTS);
		s_Renderer3DData->LightingPipeline->RegisterSB(CRC32("PointLights"), sizeof(PointLightData), MAX_NUM_POINT_LIGHTS);
		s_Renderer3DData->LightingPipeline->RegisterSB(CRC32("SpotLights"), sizeof(SpotLightData), MAX_NUM_SPOT_LIGHTS);
		
#if 0
		s_BRDFLutTexture = Texture2D::Create("Resources/Renderer/BRDF_LUT.jpg");

		s_GaussianBlurShader = pipelineLibrary.Load("assets/shaders/GaussianBlur.glsl", spec);
		s_FxaaShader = pipelineLibrary.Load("assets/shaders/FXAA.glsl", spec);
		s_HdrShader = pipelineLibrary.Load("assets/shaders/HDR.glsl", spec);
		s_BloomShader = pipelineLibrary.Load("assets/shaders/Bloom.glsl", spec);
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
		if (s_Renderer3DData->LightingPipeline->Bind(cl))
		{
			SetupLightsData(cl);
			s_Renderer3DData->LightingPipeline->SetCBData(cl, CRC32("GlobalData"), &(s_Renderer3DData->GlobalData), sizeof(GlobalData));
		}

		if (s_Renderer3DData->RenderPipeline->Bind(cl))
		{
			s_Renderer3DData->RenderPipeline->SetCBData(cl, CRC32("GlobalData"), &(s_Renderer3DData->GlobalData), sizeof(GlobalData));
		}

		if (cubemap)
		{
			if (s_Renderer3DData->CubemapPipeline->Bind(cl))
			{
				SkyLightComponent& sky = cubemap.GetComponent<SkyLightComponent>();
				const SkyboxData data
				{
					.SkyboxViewProjection = cameraData.Projection * glm::mat4(glm::mat3(cameraData.View)),
					.RotationIntensity = { sky.Rotation, sky.Intensity }
				};
				s_Renderer3DData->CubemapPipeline->SetCBData(cl, CRC32("SkyboxData"), &data, sizeof(SkyboxData));
			}
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

	static glm::mat4 CalcDirectionalLightViewProjection(const glm::mat4& worldTransform, float nearPlane, float farPlane, glm::vec4& outDirection)
	{
		glm::mat4 lightProjection = glm::orthoRH_ZO(-20.0f, 20.0f, -20.0f, 20.0f, nearPlane, farPlane);
		outDirection = worldTransform * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
		glm::vec3 pos = worldTransform[3];
		glm::vec3 dir = glm::normalize(glm::vec3(outDirection));
		glm::vec3 lookAt = pos + dir;
		glm::mat4 dirLightView = glm::lookAt(pos, lookAt, glm::vec3(0, 1, 0));
		return lightProjection * dirLightView;
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
							glm::vec4 dir(0.0f, 0.0f, 1.0f, 0.0f);
							DirectionalLightData& dirLightData = dlData[numDirectionalLights];
							dirLightData =
							{
								.Color = glm::vec4(lightComponent.Color, lightComponent.Intensity),
								.ViewProjection = CalcDirectionalLightViewProjection(worldTransform, lightComponent.NearPlane, lightComponent.FarPlane, dir),
								.QualityTextureBias = glm::vec4(lightComponent.ShadowQuality, lightComponent.ShadowMapFramebuffer->GetDepthAttachmentHeapIndex(), lightComponent.ShadowBias, 1.0f)
							};
							dirLightData.Direction = dir;
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
	}

	void Renderer3D::CompositePass([[maybe_unused]] const Ref<RenderGraphData>& renderGraphData)
	{
		ARC_PROFILE_SCOPE();

		GraphicsCommandList cl = RenderCommand::BeginRecordingCommandList();
		renderGraphData->CompositePassTarget->Bind(cl);
		renderGraphData->CompositePassTarget->Clear(cl);
		if (s_Renderer3DData->CompositePipeline->Bind(cl))
		{
			CompositeShaderProperties props =
			{
				.VignetteColor = VignetteColor,
				.VignetteOffset = VignetteOffset,
				.TonemapExposure = Exposure,
				.TonemapType = static_cast<uint32_t>(Tonemapping),

				.MainTexture = renderGraphData->LightingPassTarget->GetColorAttachmentHeapIndex(0),
				.VignetteMask = VignetteMask ? VignetteMask->GetHeapIndex() : 0xFFFFFFFF,
			};
			s_Renderer3DData->CompositePipeline->SetCBData(cl, CRC32("Properties"), &props, sizeof(CompositeShaderProperties));

			/*
			s_HdrShader->SetData("u_TonemappParams", tonemappingParams);
			s_HdrShader->SetData("u_BloomStrength", UseBloom ? BloomStrength : -1.0f);
			s_HdrShader->SetData("u_Texture", 0);
			s_HdrShader->SetData("u_BloomTexture", 1);
			s_HdrShader->SetData("u_VignetteMask", 2);

			if (UseFXAA)
				renderGraphData->FXAAPassTarget->BindColorAttachment(0, 0);
			else
				renderGraphData->LightingPassTarget->BindColorAttachment(0, 0);

			if (UseBloom)
				renderGraphData->UpsampledFramebuffers[0]->BindColorAttachment(0, 1);

			if (VignetteOffset.a > 0.0f)
			{
				s_HdrShader->SetData("u_VignetteColor", VignetteColor);
				if (VignetteMask)
					VignetteMask->Bind(2);
			}
			s_HdrShader->SetData("u_VignetteOffset", VignetteOffset);
			*/

			DrawQuad(cl);
			RenderCommand::EndRecordingCommandList(cl);
		}
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

		LightingShaderProperties lightingShaderProperties =
		{
			.AlbedoTexture = renderGraphData->RenderPassTarget->GetColorAttachmentHeapIndex(0),
			.NormalTexture = renderGraphData->RenderPassTarget->GetColorAttachmentHeapIndex(1),
			.MRATexture = renderGraphData->RenderPassTarget->GetColorAttachmentHeapIndex(2),
			.EmissiveTexture = renderGraphData->RenderPassTarget->GetColorAttachmentHeapIndex(3),
			.DepthTexture = renderGraphData->RenderPassTarget->GetDepthAttachmentHeapIndex(),
			.IrradianceTexture = AssetManager::BlackTexture()->GetHeapIndex(),

			.SkyboxIntensity = 0.0f,
			.SkyboxRotation = 0.0f,
		};

		if (s_Renderer3DData->Skylight)
		{
			const SkyLightComponent& skylightComponent = s_Renderer3DData->Skylight.GetComponent<SkyLightComponent>();
			if (skylightComponent.Texture)
			{
				lightingShaderProperties.IrradianceTexture = skylightComponent.Texture->GetIrradianceHeapIndex();
				lightingShaderProperties.SkyboxIntensity = skylightComponent.Intensity;
				lightingShaderProperties.SkyboxRotation = skylightComponent.Rotation;
			};
		}

		GraphicsCommandList cl = RenderCommand::BeginRecordingCommandList();
		renderGraphData->LightingPassTarget->Clear(cl);
		renderGraphData->LightingPassTarget->Bind(cl);
		if (s_Renderer3DData->LightingPipeline->Bind(cl))
		{
			s_Renderer3DData->LightingPipeline->BindCB(cl, CRC32("GlobalData"));
			s_Renderer3DData->LightingPipeline->SetCBData(cl, CRC32("Properties"), &lightingShaderProperties, sizeof(LightingShaderProperties));
			s_Renderer3DData->LightingPipeline->BindSB(cl, CRC32("DirectionalLights"));
			s_Renderer3DData->LightingPipeline->BindSB(cl, CRC32("PointLights"));
			s_Renderer3DData->LightingPipeline->BindSB(cl, CRC32("SpotLights"));
			DrawQuad(cl);
		}
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

			if (s_Renderer3DData->CubemapPipeline->Bind(cl))
			{
				const SkyLightComponent& skylightComponent = s_Renderer3DData->Skylight.GetComponent<SkyLightComponent>();
				if (skylightComponent.Texture)
				{
					s_Renderer3DData->CubemapPipeline->BindCB(cl, CRC32("SkyboxData"));
					skylightComponent.Texture->Bind(cl, s_Renderer3DData->CubemapPipeline->GetSlot(CRC32("EnvironmentTexture")));
					DrawCube(cl);
				}
			}
		}

		if (!s_Renderer3DData->Meshes.empty())
		{
			ARC_PROFILE_SCOPE_NAME("Draw Meshes");

			if (s_Renderer3DData->RenderPipeline->Bind(cl))
			{
				s_Renderer3DData->RenderPipeline->BindCB(cl, CRC32("GlobalData"));
				for (const MeshData& meshData : s_Renderer3DData->Meshes)
				{
					meshData.SubmeshGeometry.Mat->Bind(cl);
					s_Renderer3DData->RenderPipeline->SetRSData(cl, CRC32("Transform"), glm::value_ptr(meshData.Transform), sizeof(glm::mat4));
					RenderCommand::DrawIndexed(cl, meshData.SubmeshGeometry.Geometry);
					s_Renderer3DData->Stats.DrawCalls++;
					s_Renderer3DData->Stats.IndexCount += meshData.SubmeshGeometry.Geometry->GetIndexBuffer()->GetCount();
				}
			}
		}

		renderTarget->Unbind(cl);
		RenderCommand::EndRecordingCommandList(cl);
	}

	void Renderer3D::ShadowMapPass()
	{
		ARC_PROFILE_SCOPE();

		if (s_Renderer3DData->Meshes.empty())
			return;

		GraphicsCommandList cl = RenderCommand::BeginRecordingCommandList();
		if (s_Renderer3DData->DepthPipeline->Bind(cl))
		{
			for (const Entity lightEntity : s_Renderer3DData->SceneLights)
			{
				const LightComponent& light = lightEntity.GetComponent<LightComponent>();
				if (light.Type != LightComponent::LightType::Directional)
					continue;

				light.ShadowMapFramebuffer->Clear(cl);
				light.ShadowMapFramebuffer->Bind(cl);

				glm::mat4 worldTransform = lightEntity.GetWorldTransform();
				glm::vec4 zDir(0.0f, 0.0f, 1.0f, 0.0f);

				DepthShaderProperties properties =
				{
					.ViewProjection = CalcDirectionalLightViewProjection(worldTransform, light.NearPlane, light.FarPlane, zDir)
				};

				s_Renderer3DData->DepthPipeline->SetCBData(cl, CRC32("Properties"), &properties, sizeof(DepthShaderProperties));

				for (const MeshData& meshData : s_Renderer3DData->Meshes)
				{
					s_Renderer3DData->DepthPipeline->SetRSData(cl, CRC32("Transform"), glm::value_ptr(meshData.Transform), sizeof(glm::mat4));
					RenderCommand::DrawIndexed(cl, meshData.SubmeshGeometry.Geometry);
				}

				light.ShadowMapFramebuffer->Unbind(cl);
			}
		}

		RenderCommand::EndRecordingCommandList(cl);
	}
}
