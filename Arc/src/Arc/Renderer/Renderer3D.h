#pragma once

#include "Arc/Scene/Components.h"

namespace ArcEngine
{
	class Shader;
	class PipelineState;
	class ConstantBuffer;
	struct RenderGraphData;
	struct CameraData;
	class Entity;
	class Material;
	class VertexBuffer;
	class VertexArray;

	class Renderer3D
	{
	public:
		static constexpr uint32_t MAX_NUM_POINT_LIGHTS = 100;
		static constexpr uint32_t MAX_NUM_SPOT_LIGHTS = 100;
		static constexpr uint32_t MAX_NUM_DIR_LIGHTS = 3;

		static void Init();
		static void Shutdown();

		static void BeginScene(const CameraData& cameraData, Entity cubemap, eastl::vector<Entity>&& lights);
		static void EndScene(const Ref<RenderGraphData>& renderTarget);

		static void DrawCube();
		static void DrawQuad();
		static void SubmitMesh(glm::mat4&& transform, Ref<Material>& material, Ref<VertexArray>& geometry);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t IndexCount = 0;
		};

		static void ResetStats();
		[[nodiscard]] static Statistics GetStats();

	private:
		static void SetupGlobalData();
		static void SetupLightsData();
		static void Flush(const Ref<RenderGraphData>& renderGraphData);
		static void FXAAPass(const Ref<RenderGraphData>& renderGraphData);
		static void CompositePass(const Ref<RenderGraphData>& renderGraphData);
		static void BloomPass(const Ref<RenderGraphData>& renderGraphData);
		static void LightingPass(const Ref<RenderGraphData>& renderGraphData);
		static void RenderPass(const Ref<Framebuffer>& renderTarget);
		static void ShadowMapPass();

#if 0
	private:
		static Ref<PipelineState> s_LightingShader;
		static Ref<PipelineState> s_ShadowMapShader;
		static Ref<PipelineState> s_CubemapShader;
		static Ref<PipelineState> s_GaussianBlurShader;
		static Ref<PipelineState> s_FxaaShader;
		static Ref<PipelineState> s_HdrShader;
		static Ref<PipelineState> s_BloomShader;
		static Ref<VertexArray> s_QuadVertexArray;
		static Ref<VertexBuffer> s_CubeVertexBuffer;
		static Ref<ConstantBuffer> s_UbPointLights;
		static Ref<ConstantBuffer> s_UbDirectionalLights;

		static Entity s_Skylight;
		static eastl::vector<Entity> s_SceneLights;

	public:

		enum class TonemappingType { None = 0, ACES, Filmic, Uncharted };

		static TonemappingType Tonemapping;
		static float Exposure;
		static bool UseBloom;
		static float BloomStrength;
		static float BloomThreshold;
		static float BloomKnee;
		static float BloomClamp;
		static bool UseFXAA;
		static glm::vec2 FXAAThreshold;			// x: current threshold, y: relative threshold
		static glm::vec4 VignetteColor;			// rgb: color, a: intensity
		static glm::vec4 VignetteOffset;		// xy: offset, z: useMask, w: enable/disable effect
		static Ref<Texture2D> VignetteMask;
#endif
	};
}
