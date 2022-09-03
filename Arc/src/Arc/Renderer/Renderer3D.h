#pragma once

#include "Arc/Renderer/Camera.h"
#include "Arc/Renderer/EditorCamera.h"
#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Components.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderGraphData.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace ArcEngine
{
	class Renderer3D
	{
	public:
		const static uint32_t MAX_NUM_LIGHTS = 200;
		const static uint32_t MAX_NUM_DIR_LIGHTS = 3;

		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera,const glm::mat4& transform, Entity cubemap, eastl::vector<Entity>& lights);
		static void BeginScene(const EditorCamera& camera, Entity cubemap, eastl::vector<Entity>& lights);
		static void EndScene(const Ref<RenderGraphData>& renderTarget);

		static void DrawCube();
		static void DrawQuad();
		static void ReserveMeshes(size_t count);
		static void SubmitMesh(const MeshComponent& meshComponent, const glm::mat4& transform);

		static ShaderLibrary& GetShaderLibrary() { return s_ShaderLibrary; }

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t IndexCount = 0;
		};

		static void ResetStats();
		static Statistics GetStats();

	private:
		static void SetupCameraData();
		static void SetupLightsData();
		static void Flush(const Ref<RenderGraphData> renderGraphData);
		static void CompositePass(const Ref<RenderGraphData> renderGraphData);
		static void BloomPass(const Ref<RenderGraphData> renderGraphData);
		static void LightingPass(const Ref<RenderGraphData> renderGraphData);
		static void RenderPass(const Ref<Framebuffer> renderTarget);
		static void ShadowMapPass();

	private:
		static ShaderLibrary s_ShaderLibrary;

	public:
		enum class TonemappingType { None = 0, ACES, Filmic, Uncharted };

		static TonemappingType Tonemapping;
		static float Exposure;
		static bool UseBloom;
		static float BloomStrength;
		static float BloomThreshold;
		static float BloomKnee;
		static float BloomClamp;
	};
}
