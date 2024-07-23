#pragma once

#include "Arc/Scene/Components.h"

namespace ArcEngine
{
	struct WorkQueue;
	class Shader;
	class PipelineState;
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
		static void EndScene(WorkQueue* queue, const Ref<RenderGraphData>& renderTarget);

		static void DrawCube(GraphicsCommandList cl);
		static void DrawQuad(GraphicsCommandList cl);
		static void ReserveMeshes(size_t count);
		static void SubmitMesh(const glm::mat4& transform, Submesh& submesh);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t IndexCount = 0;
		};

		static void ResetStats();
		[[nodiscard]] static Statistics GetStats();

	private:
		static void SetupLightsData(GraphicsCommandList cl);
		static void Flush(WorkQueue* queue, const Ref<RenderGraphData>& renderGraphData);
		static void FXAAPass(const Ref<RenderGraphData>& renderGraphData);
		static void CompositePass(const Ref<RenderGraphData>& renderGraphData);
		static void BloomPass(const RenderGraphData* renderGraphData);
		static void LightingPass(const RenderGraphData* renderGraphData);
		static void RenderPass(const Ref<Framebuffer>& renderTarget);
		static void ShadowMapPass();

	public:

		enum class TonemappingType { None = 0, ACES, Filmic, Uncharted };

		static TonemappingType Tonemapping;
		static float Exposure;
		static glm::vec4 VignetteColor;			// rgb: color, a: intensity
		static glm::vec4 VignetteOffset;		// xy: offset, z: useMask, w: enable/disable effect
		static Ref<Texture2D> VignetteMask;
		static bool UseBloom;
		static float BloomStrength;
		static float BloomThreshold;
		static float BloomKnee;
		static float BloomClamp;
#if 0
		static bool UseFXAA;
		static glm::vec2 FXAAThreshold;			// x: current threshold, y: relative threshold
#endif
	};
}
